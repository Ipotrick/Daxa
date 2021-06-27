#pragma once

#include "../DaxaCore.hpp"
#include "../math/Mat.hpp"

namespace daxa {
	class FPSCamera {
	public:
		void setPosition(const Vec3& p) {
			if (p != this->position) {
				this->position = p;
				this->bViewDirty = true;
			}
		}
		const Vec3& getPosition() const { return position; }

		void setYaw(f32 y) {
			if (y != this->yaw) {
				this->yaw = y;
				this->bViewDirty = true;
			}
		}
		const f32& getYaw() const { return yaw; }

		void setPitch(f32 p) {
			if (p != this->pitch) {
				this->pitch = p;
				this->bViewDirty = true;
			}
		}
		const f32& getPitch() const { return pitch; }

		daxa::Mat4x4& getView() {
			if (bViewDirty) {
				bViewDirty = false;
				view = daxa::makeFPSView(position, pitch, yaw);
			}
			return view;
		}

		void setAspectRatio(f32 a) {
			if (a != this->aspectRatio) {
				this->aspectRatio = a;
				this->bProjDirty = true;
			}
		}
		const f32& getAspectRatio() const {
			return aspectRatio;
		}

		void setFOV(f32 angle) {
			if (angle != this->fovAsAngle) {
				this->fovAsAngle = angle;
				this->bProjDirty = true;
			}
		}
		const f32& getFOV() const {
			return this->fovAsAngle;
		}
		const daxa::Mat4x4& getProjection() {
			if (bProjDirty) {
				bProjDirty = false;
				proj = daxa::makeProjection<4, f32>(daxa::radians(fovAsAngle), aspectRatio, 0.1f, 200.0f);
			}
			return proj;
		}
	private:
		Vec3 position{ 0,0,3 };
		f32 yaw{ 0.0f };
		f32 pitch{ 0.0f };
		bool bViewDirty{ false };
		f32 aspectRatio{ 1.0f };
		f32 fovAsAngle{ 90.0f };
		bool bProjDirty{ false };
		daxa::Mat4x4 view{ 1.0f };
		daxa::Mat4x4 proj{ 1.0f };
	};
}
