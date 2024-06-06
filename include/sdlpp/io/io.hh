//
// Created by igor on 02/06/2020.
//

#ifndef NEUTRINO_SDL_IO_HH
#define NEUTRINO_SDL_IO_HH

#include <type_traits>
#include <cstdint>
#include <string>
#include <cstdio>
#include <vector>
#include <istream>
#include <ostream>
#include <filesystem>

#include "sdlpp/detail/sdl2.hh"
#include "sdlpp/detail/object.hh"
#include "sdlpp/detail/call.hh"
#include "sdlpp/io/whence.hh"
#include "bsw/mp/introspection.hh"

namespace neutrino::sdl {
	class io : public object<SDL_RWops> {
	 public:
		// Use this function to prepare a read-only memory buffer for use with RWops.
		io (const void* mem, std::size_t size);
		io (void* mem, std::size_t size);
		io (const std::string& filename, bool read_only);
		io (const std::filesystem::path& filename, bool read_only);
		io (FILE* fp, bool auto_close);

		explicit io (std::vector<char>& mem);
		explicit io (std::vector<uint8_t>& mem);
		explicit io (const std::vector<char>& mem);
		explicit io (const std::vector<uint8_t>& mem);

		io (object<SDL_RWops>&& other);
		io& operator= (object<SDL_RWops>&& other) noexcept;

		/*
		 * ptr - a pointer to a buffer to read data into
		 * size - the size of each object to read, in bytes
		 * maxnum - the maximum number of objects to be read
		 * Returns the number of objects read, or 0 at error or end of file; call SDL_GetError() for more information.
		 */
		[[nodiscard]] size_t read (void* ptr,
								   size_t size,
								   size_t maxnum);
		/*
		 * ptr - a pointer to a buffer containing data to write
		 * size - the size of an object to write, in bytes
		 * num - the number of objects to write
		 * Returns the number of objects written, which will be less than num on error; call SDL_GetError() for more information.
		 */
		[[nodiscard]] size_t write (const void* ptr,
									size_t size,
									size_t maxnum);
		// Returns the offset in the stream after seek, or throws exception
		[[nodiscard]] uint64_t seek (int64_t offset, whence w);
		// Returns the current offset in the stream, or throws exception
		[[nodiscard]] uint64_t tell ();
	};
}

// ==========================================================================================
// Implementation
// ==========================================================================================
namespace neutrino::sdl {
	inline
	io::io (const void* mem, std::size_t size)
		: object<SDL_RWops> (SAFE_SDL_CALL(SDL_RWFromConstMem, mem, static_cast<int>(size)), true) {

	}

	// --------------------------------------------------------------------------------------
	inline
	io::io (void* mem, std::size_t size)
		: object<SDL_RWops> (SAFE_SDL_CALL(SDL_RWFromMem, mem, static_cast<int>(size)), true) {

	}

	// --------------------------------------------------------------------------------------
	inline
	io::io (const std::string& filename, bool read_only)
		: object<SDL_RWops> (SAFE_SDL_CALL(SDL_RWFromFile, filename.c_str (), read_only ? "rb" : "wb"), true) {

	}

	inline
	io::io (const std::filesystem::path& filename, bool read_only)
		: object<SDL_RWops> (SAFE_SDL_CALL(SDL_RWFromFile, filename.u8string ().c_str (),
										   read_only ? "rb" : "wb"), true) {

	}

	// --------------------------------------------------------------------------------------
	inline
	io::io (FILE* fp, bool auto_close)
		: object<SDL_RWops> (SAFE_SDL_CALL(SDL_RWFromFP, fp, static_cast<SDL_bool>(auto_close)), true) {

	}

	// --------------------------------------------------------------------------------------
	inline
	io::io (std::vector<char>& mem)
		: object<SDL_RWops> (SAFE_SDL_CALL(SDL_RWFromMem, mem.data (), static_cast<int>(mem.size ())), true) {
	}

	// --------------------------------------------------------------------------------------
	inline
	io::io (std::vector<uint8_t>& mem)
		: object<SDL_RWops> (SAFE_SDL_CALL(SDL_RWFromMem, mem.data (), static_cast<int>(mem.size ())), true) {
	}

	// --------------------------------------------------------------------------------------
	inline
	io::io (const std::vector<char>& mem)
		: object<SDL_RWops> (SAFE_SDL_CALL(SDL_RWFromConstMem, mem.data (), static_cast<int>(mem.size ())), true) {
	}

	// --------------------------------------------------------------------------------------
	inline
	io::io (const std::vector<uint8_t>& mem)
		: object<SDL_RWops> (SAFE_SDL_CALL(SDL_RWFromConstMem, mem.data (), static_cast<int>(mem.size ())), true) {
	}

	// --------------------------------------------------------------------------------------
	inline
	io::io (object<SDL_RWops>&& other)
		: object<SDL_RWops> (std::move (other)) {

	}

	// --------------------------------------------------------------------------------------
	inline
	io& io::operator= (object<SDL_RWops>&& other) noexcept {
		if (this != &other) {
			object<SDL_RWops>::operator= (std::move (other));
		}
		return *this;
	}

	// --------------------------------------------------------------------------------------
	inline
	size_t io::read (void* ptr, size_t size, size_t maxnum) {
		return SDL_RWread (handle (), ptr, size, maxnum);
	}

	// --------------------------------------------------------------------------------------
	inline
	size_t io::write (const void* ptr, size_t size, size_t maxnum) {
		return SDL_RWwrite (handle (), ptr, size, maxnum);
	}

	// --------------------------------------------------------------------------------------
	inline
	uint64_t io::seek (int64_t offset, whence w) {
		return static_cast
			<uint64_t>(SAFE_SDL_CALL(SDL_RWseek, handle (), offset, static_cast<int>(w)));
	}

	// --------------------------------------------------------------------------------------
	inline
	uint64_t io::tell () {
		return static_cast<uint64_t>(SAFE_SDL_CALL(SDL_RWtell, handle ()));
	}

}

#endif
