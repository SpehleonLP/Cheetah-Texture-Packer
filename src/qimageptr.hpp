/**
 * This class was made so that we can read images from a texture atlas and
 * repack them, while minimizins file I/O
 *
 * In the end, i was too lazy to implement this. */
#if 0
#ifndef QIMAGEPTR_H
#define QIMAGEPTR_H
#include <QImageReader>
#include <QFileInfo>
#include <QImage>
#include <cassert>


class QImagePtr
{
    struct QImageRefCounted
    {
        QImageRefCounted(const QFileInfo & file) :
            m_file(file),
            m_refCount(1),
            m_loadRefCount(0)
        {
			QImageReader reader(file);
			m_format = reader.format();
			m_size   = reader.size();
        }

        QFileInfo         m_file;
        QImage            m_image;
        int               m_refCount;
        int               m_loadRefCount;

//cache some stuff so we can get it without doing fileIO
		QImage::Format    m_format;
		QSize             m_size;
    };

    QImageRefCounted * m_ptr;
    bool m_loaded;

public:
    QImagePtr() : m_ptr(0L), m_loaded(false) {}
    QImagePtr(const QFileInfo & file) : m_ptr(new QImageRefCounted), m_loaded(false) { }
    QImagePtr(QImagePtr && it) :  m_ptr(it.m_ptr), m_loaded(it.m_loaded) { it.m_ptr = 0L; it.m_loaded = false; }
    QImagePtr(const QImagePtr & it) : m_ptr(it.m_ptr), m_loaded(false)
    {
        if(m_ptr) m_ptr->m_refCount += 1;
    }

    ~QImagePtr()
    {
        if(m_ptr && m_loaded)
            Release();

        if(m_ptr && --(m_ptr->m_refCount) == 0)
            delete m_ptr;
    }

    QImagePtr & operator=(const QImagePtr & it)
    {
        m_ptr = it.m_ptr;
        if(m_ptr) m_ptr.m_refCount += 1;
        return *this;
    }
    QImagePtr & operator=(QImagePtr && it)
    {
        m_ptr = it.m_ptr;
        m_loaded = it.m_loaded;
        it.m_ptr = 0L;
        it.m_loaded = false;
        return *this;
    }

    const QFileInfo & getFile() const
    {
        assert(m_ptr);
        return m_ptr->m_file;
    }

    const QImage & getImage() const
    {
        assert(m_ptr && m_loaded == true);
        if(m_ptr->m_image.isNull())
        {
           m_ptr->m_image = QImageReader(m_ptr->m_file).read();
        }

        return m_ptr->m_image;
    }

    void AddRef()
    {
        assert(m_ptr != 0L && m_loaded == false);
        if(++(m_ptr->m_loadRefCount) == 1)
        {
            assert(m_ptr->m_image.isNull());
        }

        m_loaded = true;
    }

    void Release()
    {
        assert(m_ptr != 0L && m_loaded == true);
        if(--(m_ptr->m_loadRefCount) == 0)
        {
            m_ptr->m_image = QImage();
        }

        m_loaded = false;
    }

//get metadata without actually reading the image.
	QSize size() const
	{
		return m_ptr->m_size;
	}

	QImage::Format format() const
	{
		return m_ptr->m_format;
	}

};

#endif // QIMAGEPTR_H
#endif
