#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "../../util/PagedIndexMap.hpp"

#include "Glyph.hpp"

namespace daxa {

	struct Font {
		Font() = default;
		Font(Font&& rhs) = default;
		Font(const std::filesystem::path& d);
		~Font();
		void load(const std::filesystem::path& d);
		void unload();

		std::filesystem::path path{ "no path" };
		PagedIndexMap<Glyph> codepointToGlyph;
	private:
		static std::pair<u32, Glyph> loadGlyphFromCSVLine(std::stringstream& buffer);
	};
}