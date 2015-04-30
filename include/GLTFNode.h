/*
*
* Copyright (c) 2014, James Hurlbut
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or
* without modification, are permitted provided that the following
* conditions are met:
*
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
*
* Neither the name of James Hurlbut nor the names of its
* contributors may be used to endorse or promote products
* derived from this software without specific prior written
* permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#pragma once

#include "cinder/app/app.h"
#include "cinder/AxisAlignedBox.h"

namespace cinder {
namespace gltf {
	class Mesh;
	typedef std::shared_ptr<class Mesh>		MeshRef;
	class Node;
	typedef std::shared_ptr<class Node>		NodeRef;


	using NodeMap = std::map < std::string, NodeRef >;
	using NodeMapRef = std::shared_ptr<NodeMap>;


	class Node
	{
	public:
		static NodeRef	create() { return NodeRef(new Node()); }
		Node(){
			matrix = mat4(1);
			rotmat = mat4(1);
			scale = vec3(1);
			trans = vec3(0);
			visible = true;
		};

		std::vector<NodeRef> pChildren;
		ci::vec3 trans;
		ci::mat4 rotmat;
		ci::vec3 scale;
		ci::mat4 matrix;
		ci::vec4 worldPos;
		std::string  name;
		std::string key;
		bool	visible;
		std::vector<MeshRef> pMeshes;
		ci::AxisAlignedBox3f bounds;
		void draw(bool child, int numInstances, std::vector<mat4> rotMats, std::vector<ci::mat4> positions, std::vector<ci::mat4> scales);
	};

}
}