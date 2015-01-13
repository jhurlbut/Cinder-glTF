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

#include "GLTFNode.h"
#include "GLTFMesh.h"
#include "cinder/gl/gl.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace gl;

namespace cinder {
	namespace gltf {
		typedef vector<float>::const_iterator myiter;
		struct ordering {
			bool operator ()(pair<size_t, myiter> const& a, pair<size_t, myiter> const& b) {
				return *(a.second) > *(b.second);
			}
		};
		void Node::draw(bool child, int numInstances, std::vector<mat4> rotMats, std::vector<ci::mat4> positions, std::vector<ci::mat4> scales){

				{
					
					if (numInstances == 0){
						gl::pushModelView();
						matrix = glm::scale(scale)* rotmat;
						matrix[3] = vec4(trans,1);
						gl::multModelMatrix(matrix);

					}
					else {
						matrix = rotmat;
						rotMats.push_back(matrix);
						positions.push_back(glm::translate(trans));
						scales.push_back(glm::scale(scale));
					}
					
					//matrix[3] = vec4(trans,1);
					//gl::multModelMatrix(matrix);
					//double t1 = ci::app::getElapsedSeconds();
					//ci::app::console() << (t1 - t0) * 1000 << " : " << name << " : matrix node time " << std::endl;
					
					for (MeshRef pMesh : pMeshes)
					{
						//double t0 = ci::app::getElapsedSeconds();
						pMesh->draw(numInstances, rotMats, positions,scales);
						//double t1 = ci::app::getElapsedSeconds();
						//ci::app::console() << (t1 - t0) * 1000 << " : "<< name << " : node draw time " << std::endl;

					}
					
					std::vector<float> distances;
					for (NodeRef pChild : pChildren)
					{
						vec4 start = vec4(1);
						vec4 xform = getProjectionMatrix() * getViewMatrix() * getModelMatrix() * pChild->matrix * vec4(pChild->bounds.getCenter(), 1);
						//gl::drawColorCube(vec3(xform.x, xform.y, xform.z), vec3(.1));
						distances.push_back(xform.z);
					}
					vector<pair<size_t, myiter> > order(distances.size());
					size_t n = 0;
					for (myiter it = distances.begin(); it != distances.end(); ++it, ++n)
						order[n] = make_pair(n, it);
					
					sort(order.begin(), order.end(), ordering());
					
					//ci::app::console() << ":::::::::::" << endl;
					for (auto pDrawOrder : order)
					{
						pChildren[pDrawOrder.first]->draw(true, numInstances, rotMats, positions, scales);
					}
					if (numInstances == 0){
						gl::popModelView();
					}
				}
		};
	}
}