#include "Font.hpp"

namespace daxa {
	Font::Font(const std::filesystem::path& d) {
		load(d);
	}

	Font::~Font() {
		unload();
	}

	void Font::load(const std::filesystem::path& path) {
		std::cout << "try to load font: " << path << std::endl;
		if (std::ifstream ifs = std::ifstream(path)) {
			this->path = path;
			std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
			std::stringstream fileBuffer{ content };
			ifs.close();

			std::stringstream lineBuffer;
			std::string currLine;

			while (std::getline(fileBuffer, currLine, '\n')) {
				lineBuffer.clear();
				lineBuffer.str(currLine);

				auto [codepoint, glyph] = loadGlyphFromCSVLine(lineBuffer);
				codepointToGlyph.insert(glyph, codepoint);
			}

			std::cout << "success" << std::endl;
		}
		else {
			std::cout << "failure" << std::endl;
		}
	}

	void Font::unload() {
		codepointToGlyph.clear();
		this->path = "no path";
	}

	std::pair<u32, Glyph> Font::loadGlyphFromCSVLine(std::stringstream& buffer) {
		std::string curr;
		Glyph res;

		std::getline(buffer, curr, ',');
		u32 codepoint = std::stoi(curr);
		std::getline(buffer, curr, ',');
		res.advance = std::stof(curr);
		std::getline(buffer, curr, ',');
		res.planeBounds.left = std::stof(curr);
		std::getline(buffer, curr, ',');
		res.planeBounds.bottom = std::stof(curr);
		std::getline(buffer, curr, ',');
		res.planeBounds.right = std::stof(curr);
		std::getline(buffer, curr, ',');
		res.planeBounds.top = std::stof(curr);
		std::getline(buffer, curr, ',');
		res.atlasBounds.left = std::stof(curr);
		std::getline(buffer, curr, ',');
		res.atlasBounds.bottom = std::stof(curr);
		std::getline(buffer, curr, ',');
		res.atlasBounds.right = std::stof(curr);
		std::getline(buffer, curr, ',');
		res.atlasBounds.top = std::stof(curr);
		return { codepoint, res };
	}
}