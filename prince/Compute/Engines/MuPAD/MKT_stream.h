#ifndef __MKT_stream_h__
#define __MKT_stream_h__

/* MKT_stream.h -- simple byte streams.
 */

#include <stdio.h>
#include "MKT_linkage.h"
#include "MCO_compat.h"

/**@defgroup Byte Streams

@brief Simple byte streams mainly used for MuPAD input and output data.

 These are various classes for simple byte streams based on the abstract class
 MTstream. These streams do not use any other module of the MuPAD kernel.
 They are used in the kernel and in clients communicating with the kernel to
 store the data exchanged between kernel and clients (i.e. input and output
 of the kernel).

@see MTdata class containing formatted data for input and output, uses
    MTstream to hold the data.
*/

//@{

/** Abstract base class for uninterpreted byte streams.
 */
class MKT_DECL_LIB MTstream {
public:
    /// Stream status: #Ok or some error. #Ok is 0.
    typedef long Stat;

    /// Stream status value #Ok (=0).
    static const Stat Ok;

    /// Stream status value for unspecified errors.
    static const Stat Error;

    /// Stream status value for out of memory conditions.
    static const Stat OutOfMemory;

    /** Read stream into buffer.
     * @param buff buffer where to place the data read
     * @param buff_len number of bytes to read
     * @param bytes_read returns number of bytes actually read
     * @return status of stream */
    virtual Stat read(void* buff, size_t buff_len, size_t& bytes_read) = 0;

    /** Write buffer to stream.
     * @param data pointer to buffer containing data to write
     * @param num_bytes number of bytes to write
     * @param bytes_written returns number of bytes actually written
     * @return status of stream */
    virtual Stat write(const void* data, size_t num_bytes, size_t& bytes_written) = 0;

    /** Flush buffer after writing.
     * @return status of stream */
    virtual Stat flush() = 0;

    /** End of media reached?.
     * @return true iff end of stream is reached */
    virtual bool eom() const = 0;

    /** Copy contents from other stream.
     * @param source source stream
     * @return status of this stream */
    virtual Stat copy(MTstream* source);

    /** Set size of stream for output.
     * This is only a performance hint which may be ignored by the implementation.
     * @param size nuber of bytes which will be written
     * @return status of stream */
    virtual Stat set_size(size_t size);

    /** Get status of the stream.
     * @return status of stream */
    virtual Stat status() = 0;

    /** Get descriptive text of status.
     * The text returned might not be localized
     * (it should be in english, though).
     * @return text of status of stream */
    virtual const char* status_text();

protected:
    /** Set status, returns new value.
     * @param stat new status of stream
     * @return new status of stream */
    virtual Stat status(Stat stat) = 0;
};

/** Streams that allow random access.
 *
 * Still an abstract class.
 */
class MKT_DECL_LIB MTrandomAccessStream : public MTstream {
public:
    /** Discard n bytes since last write.
     * Moves 'stream pointer' back \a n bytes.
     * @param n number of bytes to discard
     * @return status of stream */
    virtual Stat discard(size_t n) = 0;

    /** Reset stream pointer to position where stream was created.
     * @return status of stream */
    virtual Stat rewind() = 0;
};

/** Streams stored in RAM.
 *
 * An abstract base class which nevertheless contains usefull
 * default implementations for some virtual members.
 */
class MKT_DECL_LIB MTmemoryStream : public MTrandomAccessStream {
public:
    /** Constructor.
     * One may define a fixed-size buffer to be used by the stream.
     * @param buff pointer to buffer or 0
     * @param len length of buffer, must be 0 iff \a buff is 0 */
    MTmemoryStream(unsigned char* buff, size_t len);

    /** Base address of stream buffer.
     * One may not use this pointer to modify the data directly.
     * @return pointer to buffer containing the streams data */
    virtual const void* data();

    /** Number of bytes in stream.
     * @returns number of bytes currently stored in the stream */
    virtual size_t length() const;

    virtual Stat read(void* buff, size_t buff_len, size_t& bytes_read);
    virtual Stat flush();
    virtual bool eom() const;
    virtual Stat discard(size_t n);
    virtual Stat rewind();
    virtual Stat clear();
    virtual Stat status();

protected:
    virtual Stat status(Stat);

    unsigned char* data_;
    unsigned char* end_;
    unsigned char* pos_;
    size_t size_;
    Stat stat_;
};

/** Dynamically growing stream allocated in the heap.
 *
 * The data of the stream ist stored in a dynamically growing buffer
 * allocated on the heap. The buffer is extended in chunks.
 *
 * The stream is created initially with a fixed-size static buffer.
 * Only if the data gets to large for this static buffer a
 * dynamic buffer is allocated on the heap.
 */
class MKT_DECL_LIB MTheapStream : public MTmemoryStream {
public:
    /** Create heap stream.
     * One may define an initial size and the increment by which the stream
     * buffer will grow.
     * @param size initial buffer size in bytes
     * @param incr increment (in bytes) by which the buffer is grown */
    MTheapStream(size_t size=512, size_t incr=512);

    /// Destroy hep stream.
    virtual ~MTheapStream();

    virtual Stat write(const void* data, size_t num_bytes, size_t& bytes_written);
    virtual Stat set_size(size_t size);

protected:
    unsigned char stat_buff_[512];
    unsigned char* dyn_buff_;
    size_t incr_;
};

/** Static buffer in memory used as stream.
 *
 * You have to allocate the buffer(s) containing the data of this stream. One may
 * give a buffer in the constructor or later on via the #set_buffer
 * method.
 */
class MKT_DECL_LIB MTbufferStream : public MTmemoryStream {
public:
    /** Create stream given buffer and size.
     * One may also create a stream with no initial buffer.
     * @param buffer buffer containing streams data, may be 0
     * @param len size of buffer in bytes, must be 0 iff \a buffer is 0 */
    MTbufferStream(unsigned char* buffer=0, size_t len=0);

    /** Set streams buffer address and size.
     * @param buffer buffer containing streams data
     * @param len size of buffer in bytes */
    virtual void set_buffer(unsigned char* buffer, size_t len);

    virtual Stat write(const void* data, size_t num_bytes, size_t& bytes_written);
    virtual Stat copy(MTstream* source);
    virtual Stat set_size(size_t size);
};

/** Stdio file stream.
 *
 * The streams data is stored in a file. Stdio functions are used to
 * implement the stream.
 *
 * The file must be opened before it can be used as a stream. The stream
 * does not know about the files 'name'.
 */
class MKT_DECL_LIB MTfileStream : public MTrandomAccessStream {
public:
    /** Create stream given a file pointer.
     * @param fp file pointer for file which will be used to hold the stream */
    MTfileStream(FILE* fp);

    virtual Stat write(const void* data, size_t num_bytes, size_t& bytes_written);
    virtual Stat read(void* buff, size_t buff_len, size_t& bytes_read);
    virtual Stat flush();
    virtual bool eom() const;
    virtual Stat discard(size_t n);
    virtual Stat rewind();
    virtual Stat status();
    virtual const char* status_text();

protected:
    virtual Stat status(Stat);

    FILE* file_;
    long origin_;
    int errno_;
};

/** Reads an ASCII C-string from a stream.
 * The string returned is allocated on the heap via \c new and must be deleted
 * by the caller via \c delete.
 * @param stream stream to read string from
 * @return C-string read, allocated via \c new, or 0 if the string could not
 *   be allocated */
MKT_DECL_LIB char* MTreadCString(MTstream* stream);

//@}

#endif // !__MKT_stream_h__
