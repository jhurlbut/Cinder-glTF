#pragma once

#include "cinder/app/app.h"
#include "cinder/AxisAlignedBox.h"

namespace cinder {
namespace gltf {
	class Mesh;
	typedef std::shared_ptr<class Mesh>		MeshRef;
	class Node;
	typedef std::shared_ptr<class Node>		NodeRef;
	class Node
	{
	public:
		static NodeRef	create() { return NodeRef(new Node()); }
		Node(){
			matrix = mat4(1);
			rotmat = mat4(1);
			scale = vec3(1);
			trans = vec3(0);
		};

		std::vector<NodeRef> pChildren;
		ci::vec3 trans;
		ci::mat4 rotmat;
		ci::vec3 scale;
		ci::mat4 matrix;
		std::string  name;
		std::vector<MeshRef> pMeshes;
		ci::AxisAlignedBox3f bounds;
		void draw(bool child, int numInstances, std::vector<mat4> rotMats, std::vector<ci::mat4> positions, std::vector<ci::mat4> scales);
	};

}
}