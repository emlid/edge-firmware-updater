



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
 *
 *   @(#)dd.h    8.3 (Berkeley) 4/2/94
 */

#ifndef StorageDeviceFlasher_H
#define StorageDeviceFlasher_H

#include <QObject>

/* Flags (in ddflags). */
#define C_BLOCK     0x00002
#define C_BS        0x00004
#define C_IBS       0x00080
#define C_IF        0x00100
#define C_LCASE     0x00200
#define C_NOERROR   0x00400
#define C_NOTRUNC   0x00800
#define C_OBS       0x01000
#define C_OF        0x02000
#define C_SEEK      0x04000
#define C_SYNC      0x20000
#define C_UCASE     0x40000
#define C_UNBLOCK   0x80000
#define C_OSYNC     0x100000
#define C_SPARSE    0x200000

/* Input/output stream state. */
typedef struct {
    uchar      *db;         /* buffer address */
    uchar      *dbp;        /* current buffer I/O address */
    uint64_t    dbcnt;      /* current buffer byte count */
    int64_t     dbrcnt;     /* last read byte count */
    uint64_t    dbsz;       /* buffer size */

#define ISCHR       0x01        /* character device (warn on short) */
#define ISPIPE      0x02        /* pipe (not truncatable) */
#define ISTAPE      0x04        /* tape (not seekable) */
#define NOREAD      0x08        /* not readable */
    uint       flags;

    const char  *name;      /* name */
    int         fd;         /* file descriptor */
    uint64_t    offset;     /* # of blocks to skip */
} IO;

typedef struct {
    uint64_t    in_full;    /* # of full input blocks */
    uint64_t    in_part;    /* # of partial input blocks */
    uint64_t    out_full;   /* # of full output blocks */
    uint64_t    out_part;   /* # of partial output blocks */
    uint64_t    trunc;      /* # of truncated records */
    uint64_t    swab;       /* # of odd-length swab blocks */
    uint64_t    sparse;     /* # of sparse output blocks */
    uint64_t    bytes;      /* # of bytes written */
    uint64_t    size;       /* full size of file in bytes*/
    struct timeval  start;  /* start time of dd */
} STAT;

struct FlashingParameters{
    QString blockSize;
    QString inputFile;
    QString outputFile;

    FlashingParameters(): blockSize("bs=1000000"), inputFile("if="), outputFile("of=") {}
};

class StorageDeviceFlasher;

struct arg {
    const char *name;
    void (StorageDeviceFlasher::*f)(char *);
    uint set, noset;
};

class StorageDeviceFlasher: public QObject
{
    Q_OBJECT

public:
    explicit StorageDeviceFlasher(QObject *parent = 0);
    int flashDevice(FlashingParameters);
    uint64_t getImageSize(void) {return st.size;}
    uint64_t getBytesSent(void) {return st.bytes;}
    void terminate(void);

signals:
    void updateProgress(uint32_t bytesSent, uint32_t fileSize);
    void flasherMessage(const QString& text, bool critical);

private slots:
    void flasherLog(const QString& text, bool critical = 0) {emit flasherMessage(text, critical);}

private:
    STAT st;         /* statistics */
    void dd_out(int);
    void def(void);
    void def_close(void);

    int jcl(struct FlashingParameters);
    void pos_in(void);
    void pos_out(void);
    void summary(void);
    ssize_t bwrite(int, const void *, size_t);

    static int  c_arg(const void *, const void *);
    long long strsuftoll(const char* arg, int def);
    void f_bs(char *);
    void f_if(char *);
    void f_of(char *);

    void dd_close(void);
    void dd_in(void);
    void getfdtype(IO *);
    int redup_clean_fd(int);
    int setup(void);
    off_t fsize(const char *);
    void current_summary(void);

    arg      args[];

    IO       in, out;        /* input/output state */
    uint64_t        cpy_cnt;        /* # of blocks to copy */
    off_t    pending;        /* pending seek if sparse */
    uint            ddflags;        /* conversion options */
    uint64_t        cbsz;           /* conversion block size */
    uint            files_cnt = 1;  /* # of files to copy */
    int      progress;       /* display sign of life */
    const uchar*    ctab;           /* conversion table */
    sigset_t        infoset;        /* a set blocking SIGINFO */
    bool     interrupted;


};

#endif
