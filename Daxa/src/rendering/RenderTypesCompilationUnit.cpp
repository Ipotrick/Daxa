#include "SimpleMesh.hpp"

#include <tiny_obj_loader.hpp>
#include <iostream>

namespace daxa {
	std::optional<SimpleMesh> daxa::loadMeshFromObj(const char* filename)
	{
		//attrib will contain the vertex arrays of the file
		tinyobj::attrib_t attrib;
		//shapes contains the info for each separate object in the file
		std::vector<tinyobj::shape_t> shapes;
		//materials contains the information about the material of each shape, but we won't use it.
		std::vector<tinyobj::material_t> materials;

		//error and warning output from the load function
		std::string warn;
		std::string err;



		//load the OBJ file
		tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, nullptr);
		//make sure to output the warnings to the console, in case there are issues with the file
		if (!warn.empty()) {
			std::cout << "WARN: " << warn << std::endl;
		}
		//if we have any error, print it to the console, and break the mesh loading. 
		//This happens if the file can't be found or is malformed
		if (!err.empty()) {
			std::cerr << err << std::endl;
			return {};
		}

		SimpleMesh mesh;

		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

				//hardcode loading to triangles
				int fv = 3;

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					//vertex position
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					//vertex normal
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

					//copy it into our vertex
					Vertex new_vert;
					new_vert.position.x = vx;
					new_vert.position.y = vy;
					new_vert.position.z = vz;

					new_vert.normal.x = nx;
					new_vert.normal.y = ny;
					new_vert.normal.z = nz;

					//we are setting the vertex color as the vertex normal. This is just for display purposes
					new_vert.color = new_vert.normal;

					mesh.vertices.push_back(new_vert);
				}
				index_offset += fv;
			}
		}

		return mesh;
	}
}

#include "ImageManager.hpp"

namespace daxa {
	ImageManager::Handle::Handle(ImageSlot* slot)
	{
		imgSlot = slot;
		slot->refCount++;
	}
	ImageManager::Handle::Handle(const Handle& other)
	{
		this->imgSlot = other.imgSlot;
		++this->imgSlot->refCount;
	}
	ImageManager::Handle::Handle(Handle&& other)
	{
		std::swap(this->imgSlot, other.imgSlot);
	}
	ImageManager::Handle::~Handle()
	{
		if (imgSlot) {
			--imgSlot->refCount;
		}
	}
	ReadWriteLock<vkh::Image> ImageManager::Handle::get()
	{
		assert(imgSlot);
		return imgSlot->imageMut.lock();
	}
	ReadOnlyLock<vkh::Image> ImageManager::Handle::getConst()
	{
		assert(imgSlot);
		return imgSlot->imageMut.lockReadOnly();
	}


	void ImageManager::CreateJob::execute()
	{
		std::vector<std::pair<std::string, u32>> remainingCreateQ;
		auto manager = managerMtx->lock();
		for (auto [alias, index] : manager->createQ) {
			if (auto imgOpt = manager->imageSlots[index]->imageMut.tryLock()) {
				auto& img = imgOpt.value();
				if (!img->valid()) {
					auto cmd = manager->cmdPool.getBuffer();
					auto fence = manager->fencePool.get();
					*img = vkh::loadImage(cmd, fence, alias, manager->device);
				}
			}
			else {
				remainingCreateQ.emplace_back(alias, index);
			}
		}

		manager->createQ = remainingCreateQ;
	}
}
