#include "Glyph.hpp"

Glyph scaleGlyph(Glyph g, f32 texWidth, f32 texHeight)
{
	g.atlasBounds.left /= texWidth;
	g.atlasBounds.right /= texWidth;
	g.atlasBounds.bottom /= texHeight;
	g.atlasBounds.top /= texHeight;
	return g;
}

std::ostream& operator<<(std::ostream& os, const Glyph& g)
{
	os << "{\n" <<
		"advance: " << g.advance << ",\n" <<
		"planebounds: {\n" <<
		"  left: " << g.planeBounds.left << ",\n" <<
		"  bottom: " << g.planeBounds.bottom << ",\n" <<
		"  right: " << g.planeBounds.right << ",\n" <<
		"  top: " << g.planeBounds.top << "\n},\n" <<
		"atlasbounds: {\n" <<
		"  left: " << g.atlasBounds.left << ",\n" <<
		"  bottom: " << g.atlasBounds.bottom << ",\n" <<
		"  right: " << g.atlasBounds.right << ",\n" <<
		"  top: " << g.atlasBounds.top << "\n}\n" <<
		"}";
	return os;
}
