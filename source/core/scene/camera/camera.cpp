#include "camera.hpp"
#include "rendering/film/film.hpp"

namespace camera {

Camera::Camera(film::Film* film) : film_(film) {}

Camera::~Camera() {
	delete film_;
}

film::Film& Camera::film() const {
	return *film_;
}

}
