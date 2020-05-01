FROM archlinux:latest
MAINTAINER thestr4ng3r

RUN pacman --noconfirm -Syu \
	base-devel \
	git \
	vulkan-headers \
	shaderc \
	cmake \
	vulkan-icd-loader \
	glfw-x11

CMD []
