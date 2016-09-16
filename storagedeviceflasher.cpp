/*-
 * Copyright (c) 1991, 1993, 1994
 * The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego and Lance
 * Visser of Convex Computer Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/


#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdint.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <functional>

#include <QDebug>

#include <storagedeviceflasher.h>
using namespace std;
#define NO_CONV


StorageDeviceFlasher::StorageDeviceFlasher(QObject *parent):
    QObject(parent)
{

}

off_t StorageDeviceFlasher::fsize(const char *filename) {
    struct stat fileinfo;

    if (stat(filename, &fileinfo) == 0){
        return fileinfo.st_size;
    }
    flasherLog("fsize error", true);
    return -1;
}


int StorageDeviceFlasher::flashDevice(struct FlashingParameters params) {

    ddflags = 0;
    interrupted = 0;
    progress = 1;
    int ret = jcl(params);
    if (ret){
        flasherLog("Failed to set flashing parameters", true);
        return 1;
    }

    ret = setup();
    if (ret){
        flasherLog("Failed to prepare data transfer", true);
        return 2;
    }
    if (out.name == NULL || in.name == NULL){
        flasherLog("null name", true);
        return 3;
    }

    (void)sigemptyset(&infoset);
    //(void)sigaddset(&infoset, SIGINFO);

    //(void)atexit(summary);

    if (out.name == NULL || in.name == NULL){
        flasherLog("null name", true);
        return 4;
    }
    if (strstr(out.name, "dev") == NULL /*|| strstr(in.name, ".img") == NULL*/){
        flasherLog("broken name", true);
        return 5;
    }

    emit startFlashing();
    while (files_cnt--){
        dd_in();
    }
    dd_close();

    progress = 0;
    return 0;
}

int StorageDeviceFlasher::setup(void) {
    if (in.name == NULL) {
        in.name = "stdin";
        in.fd = STDIN_FILENO;
    } else {
        in.fd = open(in.name, O_RDONLY, 0);
        if (in.fd < 0) {
            flasherLog(QString("%1: cannot open for read: %2").arg(in.name).arg(strerror(errno)), true);
            return 1;

        }
        st.size = (uint64_t) fsize(in.name);

        /* Ensure in.fd is outside the stdio descriptor range */
        in.fd = redup_clean_fd(in.fd);
    }

    getfdtype(&in);

    if (files_cnt > 1 && !(in.flags & ISTAPE)) {
        flasherLog("files is not supported for non-tape devices", true);
        return 1;
    }

    if (out.name == NULL) {
        /* No way to check for read access here. */
        out.fd = STDOUT_FILENO;
        out.name = "stdout";
    } else {
#define OFLAGS \
    (O_CREAT | (ddflags & (C_SEEK | C_NOTRUNC) ? 0 : O_TRUNC))
        out.fd = open(out.name, O_RDWR | OFLAGS /*, DEFFILEMODE */);
        /*
         * May not have read access, so try again with write only.
         * Without read we may have a problem if output also does
         * not support seeks.
         */
        if (out.fd < 0) {
            out.fd = open(out.name, O_WRONLY | OFLAGS /*, DEFFILEMODE */);
            out.flags |= NOREAD;
        }
        if (out.fd < 0) {
            flasherLog(QString("%1: cannot open for write: %2").arg(out.name).arg(strerror(errno)), true);
            return 1;
        }

        /* Ensure out.fd is outside the stdio descriptor range */
        out.fd = redup_clean_fd(out.fd);
    }

    getfdtype(&out);

    /*
         * Allocate space for the input and output buffers.  If not doing
         * record oriented I/O, only need a single buffer.
         */
    if (!(ddflags & (C_BLOCK|C_UNBLOCK))) {
        if ((in.db = (uchar *)malloc(out.dbsz + in.dbsz - 1)) == NULL) {
            flasherLog("Failed to allocate single buffer for input and output", true);
            return 1;
        }
        out.db = in.db;
    } else if ((in.db = (uchar *)malloc((uint)(MAX(in.dbsz, cbsz) + cbsz))) == NULL ||
               (out.db = (uchar *)malloc((uint)(out.dbsz + cbsz))) == NULL) {
        flasherLog("Failed to allocate space for input or output buffer", true);
        return 1;
    }
    in.dbp = in.db;
    out.dbp = out.db;

    /* Position the input/output streams. */
    if (in.offset)
        pos_in();
    if (out.offset)
        pos_out();

    /*
         * Truncate the output file; ignore errors because it fails on some
             * kinds of output files, tapes, for example.
                 */
    if ((ddflags & (C_OF | C_SEEK | C_NOTRUNC)) == (C_OF | C_SEEK))
        (void)ftruncate(out.fd, (off_t)out.offset * out.dbsz);

    /*
         * If converting case at the same time as another conversion, build a
             * table that does both at once.  If just converting case, use the
                 * built-in tables.
                     */
    if (ddflags & (C_LCASE|C_UCASE)) {
        flasherLog("case conv and -DNO_CONV", true);
        return 1;

    }

    (void)gettimeofday(&st.start, NULL);    /* Statistics timestamp. */

    return 0;
}

void StorageDeviceFlasher::getfdtype(IO *io) {
    //  struct mtget mt;
    struct stat sb;

    if (fstat(io->fd, &sb)) {
        flasherLog(QString("%1: cannot fstat: %2").arg(io->name).arg(strerror(errno)), true);
        terminate();
        return;
    }
    if (S_ISCHR(sb.st_mode))
        io->flags |= /*ioctl(io->fd, MTIOCGET, &mt) ? ISCHR : ISTAPE; */ ISCHR;
    else if (lseek(io->fd, (off_t)0, SEEK_CUR) == -1 && errno == ESPIPE)
        io->flags |= ISPIPE;        /* XXX fixed in 4.4BSD */
}

/*
 * Move the parameter file descriptor to a descriptor that is outside the
* stdio descriptor range, if necessary.  This is required to avoid
* accidentally outputting completion or error messages into the
* output file that were intended for the tty.
*/
int StorageDeviceFlasher::redup_clean_fd(int fd) {
    int newfd;

    if (fd != STDIN_FILENO && fd != STDOUT_FILENO &&
            fd != STDERR_FILENO)
        /* File descriptor is ok, return immediately. */
        return fd;

    /*
    * 3 is the first descriptor greater than STD*_FILENO.  Any
    * free descriptor valued 3 or above is acceptable...
    */
    newfd = fcntl(fd, F_DUPFD, 3); //duplicates file descriptor

    if (newfd < 0) {
        flasherLog(QString("dupfd IO: %1").arg(strerror(errno)), true);
        terminate();
    }

    close(fd);

    return newfd;
}

void StorageDeviceFlasher::dd_in(void) {
    int flags;
    int64_t n;

    for (flags = ddflags;;) {
        if (interrupted == 1) {
            return;
        }

        if (cpy_cnt && (st.in_full + st.in_part) >= cpy_cnt)
            return;

        /*
        * Clear the buffer first if doing "sync" on input.
        * If doing block operations use spaces.  This will
        * affect not only the C_NOERROR case, but also the
        * last partial input block which should be padded
        * with zero and not garbage.
        */
        if (flags & C_SYNC) {
            if (flags & (C_BLOCK|C_UNBLOCK))
                (void)memset(in.dbp, ' ', in.dbsz);
            else
                (void)memset(in.dbp, 0, in.dbsz);
        }

        n = read(in.fd, in.dbp, in.dbsz);
        if (n == 0) {
            in.dbrcnt = 0;
            return;
        }

        /* Read error. */
        if (n < 0) {

            /*
             * If noerror not specified, die.  POSIX requires that
             * the warning message be followed by an I/O display.
             */
            flasherLog(QString("%1: read error: %2").arg(in.name).arg(strerror(errno)), true);
            if (!(flags & C_NOERROR)) {
                terminate();
                return;
            }
            summary();

            /*
             * If it's not a tape drive or a pipe, seek past the
             * error.  If your OS doesn't do the right thing for
             * raw disks this section should be modified to re-read
             * in sector size chunks.
             */
            if (!(in.flags & (ISPIPE|ISTAPE)) &&
                    lseek(in.fd, (off_t)in.dbsz, SEEK_CUR))
                flasherLog(QString("%1: seek error: %2").arg(in.name).arg(strerror(errno)), true);

            /* If sync not specified, omit block and continue. */
            if (!(ddflags & C_SYNC))
                continue;

            /* Read errors count as full blocks. */
            in.dbcnt += in.dbrcnt = in.dbsz;
            ++st.in_full;

            /* Handle full input blocks. */
        } else if ((uint64_t)n == in.dbsz) {
            in.dbcnt += in.dbrcnt = n;
            ++st.in_full;

            /* Handle partial input blocks. */
        } else {
            /* If sync, use the entire block. */
            if (ddflags & C_SYNC)
                in.dbcnt += in.dbrcnt = in.dbsz;
            else
                in.dbcnt += in.dbrcnt = n;
            ++st.in_part;
        }

        /*
         * POSIX states that if bs is set and no other conversions
         * than noerror, notrunc or sync are specified, the block
         * is output without buffering as it is read.
         */
        if (ddflags & C_BS) {
            out.dbcnt = in.dbcnt;
            dd_out(1);

            in.dbcnt = 0;
            continue;
        }

        /*      if (ddflags & C_SWAB) {dd_in
                    if ((n = in.dbrcnt) & 1) {
                        ++st.swab;
                        --n;
                    }
                    swab(in.dbp, in.dbp, n);
                 }
                */
        in.dbp += in.dbrcnt;
        //(*cfunc)();
        def();
    }
}

/*
 * Cleanup any remaining I/O and flush output.  If necesssary, output file
 * is truncated.
 */
void StorageDeviceFlasher::dd_close(void) {

    //if (cfunc == def)
        def_close();

    if (ddflags & C_OSYNC && out.dbcnt < out.dbsz) {
        (void)memset(out.dbp, 0, out.dbsz - out.dbcnt);
        out.dbcnt = out.dbsz;
    }
    /* If there are pending sparse blocks, make sure
     * to write out the final block un-sparse
     */
    if ((out.dbcnt == 0) && pending) {
        memset(out.db, 0, out.dbsz);
        out.dbcnt = out.dbsz;
        out.dbp = out.db + out.dbcnt;
        pending -= out.dbsz;
    }
    if (out.dbcnt)
        dd_out(1);

    /*
     * Reporting nfs write error may be defered until next
     * write(2) or close(2) system call.  So, we need to do an
     * extra check.  If an output is stdout, the file structure
     * may be shared among with other processes and close(2) just
     * decreases the reference count.
     */
    if (out.fd == STDOUT_FILENO && fsync(out.fd) == -1 && errno != EINVAL) { //linux

        flasherLog(QString("fsync stdout: %1").arg(strerror(errno)), true);
        terminate();
        return;
    }
    if (close(out.fd) == -1) {
        flasherLog(QString("close: %1").arg(strerror(errno)), true);
        terminate();
        return;
    }

    if (out. db != in.db){
        free(out.db);
    }
    free(in.db);
}

void StorageDeviceFlasher::dd_out(int force) {
    static int warned;
    int64_t cnt, n, nw;
    uchar *outp;

    /*
     * Write one or more blocks out.  The common case is writing a full
     * output block in a single write; increment the full block stats.
     * Otherwise, we're into partial block writes.  If a partial write,
     * and it's a character device, just warn.  If a tape device, quit.
     *
     * The partial writes represent two cases.  1: Where the input block
     * was less than expected so the output block was less than expected.
     * 2: Where the input block was the right size but we were forced to
     * write the block in multiple chunks.  The original versions of dd(1)
     * never wrote a block in more than a single write, so the latter case
     * never happened.
     *
     * One special case is if we're forced to do the write -- in that case
     * we play games with the buffer size, and it's usually a partial write.
     */
    outp = out.db;
    for (n = force ? out.dbcnt : out.dbsz;; n = out.dbsz) {
        for (cnt = n;; cnt -= nw) {

            if (!force && ddflags & C_SPARSE) {
                int sparse, i;
                sparse = 1; /* Is buffer sparse? */
                for (i = 0; i < cnt; i++)
                    if (outp[i] != 0) {
                        sparse = 0;
                        break;
                    }
                if (sparse) {
                    pending += cnt;
                    outp += cnt;
                    nw = 0;
                    break;
                }
            }
            if (pending != 0) {
                if (lseek(out.fd, pending, SEEK_CUR) ==
                        -1) {
                    flasherLog(QString("%1: seek error creating sparse file: %2").arg(out.name).arg(strerror(errno)), true);
                    terminate();
                    return;
                }
            }
            nw = bwrite(out.fd, outp, cnt);
            if (nw <= 0) {
                if (nw == 0) {
                    flasherLog(QString("%1: end of device").arg(out.name), true);
                    terminate();
                    return;
                }
                if (errno != EINTR) {
                    flasherLog(QString("%1: write error: %2").arg(out.name).arg(strerror(errno)), true);
                    terminate();
                    return;
                }
                nw = 0;
            }
            if (pending) {
                st.bytes += pending;
                st.sparse += pending/out.dbsz;
                st.out_full += pending/out.dbsz;
                pending = 0;
            }
            outp += nw;
            st.bytes += nw;
            if (nw == n) {
                if ((uint64_t)n != out.dbsz)
                    ++st.out_part;
                else
                    ++st.out_full;
                break;
            }
            ++st.out_part;
            if (nw == cnt)
                break;
            if (out.flags & ISCHR && !warned) {
                warned = 1;
                flasherLog(QString("%1: short write on character " "device").arg(out.name), true);
            }
            if (out.flags & ISTAPE) {
                flasherLog(QString("%1: short write on tape device").arg(out.name), true);
                terminate();
                return;
            }
        }
        if ((out.dbcnt -= n) < out.dbsz)
            break;
    }

    /* Reassemble the output block. */
    if (out.dbcnt)
        (void)memmove(out.db, out.dbp - out.dbcnt, out.dbcnt);
    out.dbp = out.db + out.dbcnt;

    if (progress == 1 && interrupted == 0) {
        current_summary();
        emit updateProgress(st.bytes, st.size);
    }
}
/*
 * A protected against SIGINFO write
 */
ssize_t StorageDeviceFlasher::bwrite(int fd, const void *buf, size_t len) {
    sigset_t oset;
    ssize_t rv;
    int oerrno;

    (void)sigprocmask(SIG_BLOCK, &infoset, &oset);
    rv = write(fd, buf, len);
    oerrno = errno;
    (void)sigprocmask(SIG_SETMASK, &oset, NULL);
    errno = oerrno;
    return (rv);
}

/*
 * Position input/output data streams before starting the copy.  Device type
 * dependent.  Seekable devices use lseek, and the rest position by reading.
 * Seeking past the end of file can cause null blocks to be written to the
 * output.
 */
void StorageDeviceFlasher::pos_in(void) {
    int bcnt, cnt, nr, warned;

    /* If not a pipe or tape device, try to seek on it. */
    if (!(in.flags & (ISPIPE|ISTAPE))) {
        if (lseek(in.fd,
                  (off_t)in.offset * (off_t)in.dbsz, SEEK_CUR) == -1) {
            flasherLog(QString("%1: seek error: %2").arg(in.name).arg(strerror(errno)), true);
            terminate();
            return;
        }
        return;
        // NOTREACHED
    }

    /*
     * Read the data.  If a pipe, read until satisfy the number of bytes
     * being skipped.  No differentiation for reading complete and partial
     * blocks for other devices.
     */
    for (bcnt = in.dbsz, cnt = in.offset, warned = 0; cnt;) {
        if ((nr = read(in.fd, in.db, bcnt)) > 0) {
            if (in.flags & ISPIPE) {
                if (!(bcnt -= nr)) {
                    bcnt = in.dbsz;
                    --cnt;
                }
            } else
                --cnt;
            continue;
        }

        if (nr == 0) {
            if (files_cnt > 1) {
                --files_cnt;
                continue;
            }
            flasherLog("skip reached end of input", true);
            terminate();
            return;
        }

        /*
         * Input error -- either EOF with no more files, or I/O error.
         * If noerror not set die.  POSIX requires that the warning
         * message be followed by an I/O display.
         */
        if (ddflags & C_NOERROR) {
            if (!warned) {

                flasherLog(QString("%1: error occurred").arg(in.name), true);
                warned = 1;
                summary();
            }
            continue;
        }
        flasherLog(QString("%1: read error: %2").arg(in.name).arg(strerror(errno)), true);
        terminate();
        return;
    }
}

void StorageDeviceFlasher::pos_out(void) {
    //  struct mtop t_op;
    uint64_t cnt;
    int n;

    /*
     * If not a tape, try seeking on the file.  Seeking on a pipe is
     * going to fail, but don't protect the user -- they shouldn't
     * have specified the seek operand.
     */
    if (!(out.flags & ISTAPE)) {
        if (lseek(out.fd,
                  (off_t)out.offset * (off_t)out.dbsz, SEEK_SET) == -1) {
            flasherLog(QString("%1: seek error: %2").arg(out.name).arg(strerror(errno)), true);
            terminate();
            return;
        }
        return;
    }

    /* If no read access, try using mtio. */
    if (out.flags & NOREAD) {
        /*      t_op.mt_op = MTFSR;
                t_op.mt_count = out.offset;

                        if (ioctl(out.fd, MTIOCTOP, &t_op) < 0)*/
        flasherLog(QString("%1: cannot read").arg(out.name), true);
        terminate();
        return;
    }

    /* Read it. */
    for (cnt = 0; cnt < out.offset; ++cnt) {
        if ((n = read(out.fd, out.db, out.dbsz)) > 0)
            continue;

        if (n < 0) {
            flasherLog(QString("%1: cannot position by reading: %2").arg(out.name).arg(strerror(errno)), true);
            terminate();
            return;
        }

        /*
         * If reach EOF, fill with NUL characters; first, back up over
         * the EOF mark.  Note, cnt has not yet been incremented, so
         * the EOF read does not count as a seek'd block.
         */
        /*     t_op.mt_op = MTBSR;
               t_op.mt_count = 1;
                       if (ioctl(out.fd, MTIOCTOP, &t_op) == -1) */ {
            flasherLog(QString("%1: cannot position").arg(out.name), true);
            terminate();
            return;
        }

        while (cnt++ < out.offset)
            if ((n = bwrite(out.fd, out.db, out.dbsz)) != (long int)out.dbsz) {
                flasherLog(QString("%1: cannot position by writing: %2").arg(out.name).arg(strerror(errno)), true);
                terminate();
                return;
            }
        break;
    }
}

/*
 * def --
 * Copy input to output.  Input is buffered until reaches obs, and then
 * output until less than obs remains.  Only a single buffer is used.
 * Worst case buffer calculation is (ibs + obs - 1).
 */
void StorageDeviceFlasher::def(void) {
    uint64_t cnt;
    uchar *inp;
    const uchar *t;

    if ((t = ctab) != NULL)
        for (inp = in.dbp - (cnt = in.dbrcnt); cnt--; ++inp)
            *inp = t[*inp];

    /* Make the output buffer look right. */
    out.dbp = in.dbp;
    out.dbcnt = in.dbcnt;

    if (in.dbcnt >= out.dbsz) {
        /* If the output buffer is full, write it. */
        dd_out(0);

        /*
         * Ddout copies the leftover output to the beginning of
         * the buffer and resets the output buffer.  Reset the
         * input buffer to match it.
         */
        in.dbp = out.dbp;
        in.dbcnt = out.dbcnt;
    }
}

void StorageDeviceFlasher::def_close(void) {
    /* Just update the count, everything is already in the buffer. */
    if (in.dbcnt)
        out.dbcnt = in.dbcnt;
}




#define tv2mS(tv) ((tv).tv_sec * 1000LL + ((tv).tv_usec + 500) / 1000)

void StorageDeviceFlasher::summary(void) {

    char buf[100];
    int64_t mS;
    struct timeval tv;

    if (!progress && !interrupted){
        (void)snprintf(buf, sizeof(buf), "\r [100%%] %ld blocks (%ld MB) written.",
                       (long)st.out_full, (long) st.bytes/1000000);
        (void)write(STDERR_FILENO, buf, strlen(buf));
        (void)snprintf(buf, sizeof(buf), "\nFlashing is finished successfully");
        (void)write(STDERR_FILENO, buf, strlen(buf));
    } else {
        current_summary();
    }


    (void)write(STDERR_FILENO, "\n", 1);

   (void)gettimeofday(&tv, NULL); //linux

    mS = tv2mS(tv) - tv2mS(st.start);

    if (mS == 0)
        mS = 1;
    /* Use snprintf(3) so that we don't reenter stdio(3). */
    (void)snprintf(buf, sizeof(buf),
                   "%llu+%llu records in\n%llu+%llu records out\n",
                   (unsigned long long)st.in_full,  (unsigned long long)st.in_part,
                   (unsigned long long)st.out_full, (unsigned long long)st.out_part);
    (void)write(STDERR_FILENO, buf, strlen(buf));
    if (st.swab) {
        (void)snprintf(buf, sizeof(buf), "%llu odd length swab %s\n",
                       (unsigned long long)st.swab,
                       (st.swab == 1) ? "block" : "blocks");
        (void)write(STDERR_FILENO, buf, strlen(buf));
    }
    if (st.trunc) {
        (void)snprintf(buf, sizeof(buf), "%llu truncated %s\n",
                       (unsigned long long)st.trunc,
                       (st.trunc == 1) ? "block" : "blocks");
        (void)write(STDERR_FILENO, buf, strlen(buf));
    }
    if (st.sparse) {
        (void)snprintf(buf, sizeof(buf), "%llu sparse output %s\n",
                       (unsigned long long)st.sparse,
                       (st.sparse == 1) ? "block" : "blocks");
        (void)write(STDERR_FILENO, buf, strlen(buf));
    }
    (void)snprintf(buf, sizeof(buf),
                   "%llu bytes (%4.1f MB) transferred in %lu.%03d s (%4.1f MB/s)\n",
                   (unsigned long long) st.bytes,
                   (unsigned long long) st.bytes / 1e6,
                   (long) (mS / 1000),
                   (int) (mS % 1000),
                   (double) (st.bytes / 1e3 / mS));
    (void)write(STDERR_FILENO, buf, strlen(buf));


}



void StorageDeviceFlasher::current_summary(void) {
    char buf[100];
    long int blocks = st.size / out.dbsz;
    long int percent = (blocks % 100) == 0 ? blocks/100 : blocks/100+1;
    if ((st.out_full % (percent * 1)) == 0){
        (void)snprintf(buf, sizeof(buf), "\r [%ld%%] %ld blocks (%ld MB) written.",
                       (long) st.out_full / percent, (long)st.out_full, (long) st.bytes/1000000);
        (void)write(STDERR_FILENO, buf, strlen(buf));
    }
}


void StorageDeviceFlasher::terminate(bool cancel) {
    if (!cancel) {
        flasherLog("Flashing Failed due to error");
    }
    interrupted = 1;
    files_cnt = 0;
}


int StorageDeviceFlasher::jcl(struct FlashingParameters fp) {
    /*
     * args -- parse JCL syntax of dd.
      */
    arg /*StorageDeviceFlasher::*/args[3] = {
        { "bs",     &StorageDeviceFlasher::f_bs,       C_BS,    C_BS|C_IBS|C_OBS|C_OSYNC }, //read and write up to BYTES bytes at a time
        { "if",     &StorageDeviceFlasher::f_if,       C_IF,    C_IF },//read from FILE instead of stdin
        { "of",     &StorageDeviceFlasher::f_of,       C_OF,    C_OF } //write to FILE instead of stdout

    };


    struct arg *ap, tmp;

    QByteArray operand;
    QByteArray value;
    QList<QString> parameters;
    parameters.append(fp.blockSize);
    parameters.append(fp.inputFile);
    parameters.append(fp.outputFile);

    int n = parameters.size();
    in.dbsz = out.dbsz = 512; //default

    for (int i = 0; i < n; i++) {

        operand = parameters.at(i).left(2).toLatin1();
        value = parameters.at(i).section('=',-1).toLocal8Bit();

        tmp.name = operand.data();

        if (!(ap = (struct arg *)bsearch(&tmp, args, sizeof(args)/sizeof(struct arg), sizeof(struct arg), c_arg))) {
            flasherLog(QString("unknown operand %1").arg(tmp.name), true);
            return 1;
        }

        if (ddflags & ap->noset) {
            flasherLog(QString("%1: illegal argument combination or already set").arg(tmp.name), true);
            return 1;
        }
        ddflags |= ap->set;
       (this->* ap->f)(value.data());
    }
    //cfunc = def;
    return 0;
}

int StorageDeviceFlasher::c_arg(const void *a, const void *b) {

    return (strcmp(((const struct arg *)a)->name,
                   ((const struct arg *)b)->name));
}

long long StorageDeviceFlasher::strsuftoll(const char* arg, int def) {
    long long result;

    if (sscanf(arg, "%lld", &result) == 0)
        result = (long long)def;
    return result;
}

void StorageDeviceFlasher::f_bs(char *arg) {
    in.dbsz = out.dbsz = strsuftoll(arg, 1);
}

void StorageDeviceFlasher::f_if(char *arg) {
    in.name = arg;
}

void StorageDeviceFlasher::f_of(char *arg) {
    out.name = arg;
}
