#pragma once

#include <stdio.h>

#include "../dependencies/stb_image.hpp"
#include "../dependencies/tiny_obj_loader.hpp"

#include "../src/DaxaCore.hpp"

#include "../src/math/Rota2.hpp"
#include "../src/math/Mat.hpp"

#include "../src/threading/OwningMutex.hpp"
#include "../src/threading/Jobs.hpp"

#include "../src/DaxaApplication.hpp"

namespace daxa 
{
	void initialize();

	void cleanup();
}
