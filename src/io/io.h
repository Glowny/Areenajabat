#include <bx/bx.h>
#include <bx/readerwriter.h>

namespace arena
{
    class FileReader : public bx::FileReaderI
    {
    public:
        FileReader()
            : m_file(NULL)
        {
        }

        virtual ~FileReader()
        {
        }

        virtual bool open(const char* _filePath, bx::Error* _err) BX_OVERRIDE
        {
            BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

            m_file = fopen(_filePath, "rb");
            if (NULL == m_file)
            {
                BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "CrtFileReader: Failed to open file.");
                return false;
            }

            return true;
        }

        virtual void close() BX_OVERRIDE
        {
            fclose(m_file);
        }

        virtual int64_t seek(int64_t _offset = 0, bx::Whence::Enum _whence = bx::Whence::Current) BX_OVERRIDE
        {
            fseeko64(m_file, _offset, _whence);
            return ftello64(m_file);
        }

        virtual int32_t read(void* _data, int32_t _size, bx::Error* _err) BX_OVERRIDE
        {
            BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

            int32_t size = (int32_t)fread(_data, 1, _size, m_file);
            if (size != _size)
            {
                BX_ERROR_SET(_err, BX_ERROR_READERWRITER_READ, "CrtFileReader: read failed.");
                return size >= 0 ? size : 0;
            }

            return size;
        }

    private:
        FILE* m_file;
    };

    class FileWriter : public bx::FileWriterI
    {
    public:
        FileWriter()
            : m_file(NULL)
        {
        }

        virtual ~FileWriter()
        {
        }

        virtual bool open(const char* _filePath, bool _append, bx::Error* _err) BX_OVERRIDE
        {
            m_file = fopen(_filePath, _append ? "ab" : "wb");

            if (NULL == m_file)
            {
                BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "CrtFileWriter: Failed to open file.");
                return false;
            }

            return true;
        }

        virtual void close() BX_OVERRIDE
        {
            fclose(m_file);
        }

        virtual int64_t seek(int64_t _offset = 0, bx::Whence::Enum _whence = bx::Whence::Current) BX_OVERRIDE
        {
            fseeko64(m_file, _offset, _whence);
            return ftello64(m_file);
        }

        virtual int32_t write(const void* _data, int32_t _size, bx::Error* _err) BX_OVERRIDE
        {
            BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

            int32_t size = (int32_t)fwrite(_data, 1, _size, m_file);
            if (size != _size)
            {
                BX_ERROR_SET(_err, BX_ERROR_READERWRITER_WRITE, "CrtFileWriter: write failed.");
                return size >= 0 ? size : 0;
            }

            return size;
        }

    private:
        FILE* m_file;
    };
}