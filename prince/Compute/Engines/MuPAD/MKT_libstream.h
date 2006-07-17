#ifndef __MKT_libstream_h__
#define __MKT_libstream_h__

#include "MKT_stream.h"
#include "MCO_compat.h"

class MCLT_iostream;

/** Read-only stream for files and MuPAD library tar file entries.
 *
 * This stream may open (stdio) files and files contained in a MuPAD library
 * tar file (tar file entries). It is mainly used by debugger front-ends to
 * read the sources of the MuPAD library.
 */
class MTlibraryStream : public MTrandomAccessStream {
public:
    /** Create library stream.
     * Must call #open() before reading. */
    MTlibraryStream();

    /// Destructor, closes the stream.
    virtual ~MTlibraryStream();

    /** Open the named file or tar file entry for reading.
     * @param name file name or name of tar file entry in locale encoding
     * @return status of stream */
    virtual Stat open(const char* name);

    /** Close the file or tar file entry.
     * @return status of stream */
    virtual Stat close();

    /** Get length of files content.
     * @return length of file or tar file entry (in bytes) */
    size_t length();

    /// Raises an error (stream is read-only).
    virtual Stat write(const void* data, size_t num_bytes, size_t& bytes_written);

    /// Raises an error (stream is read-only).
    virtual Stat copy(MTstream* source);

    virtual Stat read(void* buff, size_t buff_len, size_t& bytes_read);
    virtual Stat flush();
    virtual bool eom() const;
    virtual Stat discard(size_t n);
    virtual Stat rewind();
    virtual Stat status();

protected:
    virtual Stat status(Stat);

    Stat stat_;
    MCLT_iostream* str_;
};

#endif // !__MKT_libstream_h__
