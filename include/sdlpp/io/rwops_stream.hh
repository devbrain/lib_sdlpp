//
// Created by igor on 6/6/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_IO_RWOPS_STREAM_HH_
#define SDLPP_INCLUDE_SDLPP_IO_RWOPS_STREAM_HH_

#include <sdlpp/io/rwops.hh>

namespace neutrino::sdl {
	/**
	 * @class rw_ostream
	 * @brief A class that provides write operations on an std::ostream object.
	 *
	 * The rw_ostream class is a derived class of rwops_base and provides write operations on an std::ostream object.
	 */
	class rw_ostream : public rwops_base<rw_ostream> {
	 public:
		explicit rw_ostream (std::ostream& is);

		[[nodiscard]] size_t write (const void* ptr,
									size_t size,
									size_t maxnum);
		/*
		 * ptr - a pointer to a buffer containing data to write
		 * size - the size of an object to write, in bytes
		 * num - the number of objects to write
		 * Returns the number of objects written, which will be less than num on error; call SDL_GetError() for more information.
		 */
		// Returns the offset in the stream after seek, or throws exception
		[[nodiscard]] int64_t seek (int64_t offset, whence w);
	 private:
		std::ostream& m_ostream;
	};

	/**
	 * @class rw_istream
	 *
	 * @brief Class representing an input stream for reading data.
	 *
	 * The rw_istream class is used to read data from an input stream. It is derived from the rwops_base class,
	 * which provides the underlying functionality for stream operations.
	 */
	class rw_istream : public rwops_base<rw_istream> {
	 public:
		explicit rw_istream (std::istream& is);

		[[nodiscard]] size_t read (void* ptr,
								   size_t size,
								   size_t maxnum);
		/*
		 * ptr - a pointer to a buffer containing data to write
		 * size - the size of an object to write, in bytes
		 * num - the number of objects to write
		 * Returns the number of objects written, which will be less than num on error; call SDL_GetError() for more information.
		 */
		// Returns the offset in the stream after seek, or throws exception
		[[nodiscard]] int64_t seek (int64_t offset, whence w);
		// Returns the current offset in the stream, or throws exception
		[[nodiscard]] int64_t size ();
	 private:
		std::istream& m_istream;
	};

	inline
	rw_istream::rw_istream (std::istream& is)
		: rwops_base<rw_istream> (),
		  m_istream (is) {

	}

// ---------------------------------------------------------------------
	inline
	size_t rw_istream::read (void* ptr,
							 size_t size,
							 size_t maxnum) {
		if (size == 0) { return -1; }

		m_istream.read ((char*)ptr, size * maxnum);

		return m_istream.bad () ? -1 : m_istream.gcount () / size;
	}

// -------------------------------------------------------------------------
	inline
	int64_t rw_istream::seek (int64_t offset, whence w) {
		if (w == whence::SET) {
			m_istream.seekg (offset, std::ios::beg);
		} else if (w == whence::CUR) {
			m_istream.seekg (offset, std::ios::cur);
		} else if (w == whence::END) {
			m_istream.seekg (offset, std::ios::end);
		}

		return m_istream.fail () ? (int64_t)-1 : (int64_t)
			m_istream.tellg ();
	}

// -------------------------------------------------------------------------
	inline
	int64_t rw_istream::size () {
		auto cur = m_istream.tellg ();
		m_istream.seekg (0, std::ios::end);
		auto end = m_istream.tellg ();
		m_istream.seekg (cur, std::ios::cur);
		if (m_istream.fail ()) {
			return -1;
		}
		return end;
	}

// =========================================================================
	inline
	rw_ostream::rw_ostream (std::ostream& is)
		: rwops_base<rw_ostream> (),
		  m_ostream (is) {

	}

// ---------------------------------------------------------------------
	inline
	size_t rw_ostream::write (const void* ptr,
							  size_t size,
							  size_t maxnum) {
		if (size == 0) { return -1; }
		auto before = m_ostream.tellp ();
		m_ostream.write ((char*)ptr, size * maxnum);
		auto after = m_ostream.tellp ();

		return m_ostream.bad () ? -1 : (after - before) / size;
	}

// -------------------------------------------------------------------------
	inline
	int64_t rw_ostream::seek (int64_t offset, whence w) {
		if (w == whence::SET) {
			m_ostream.seekp (offset, std::ios::beg);
		} else if (w == whence::CUR) {
			m_ostream.seekp (offset, std::ios::cur);
		} else if (w == whence::END) {
			m_ostream.seekp (offset, std::ios::end);
		}

		return m_ostream.fail () ? (int64_t)-1 : (int64_t)
			m_ostream.tellp ();
	}

}

#endif //SDLPP_INCLUDE_SDLPP_IO_RWOPS_STREAM_HH_
