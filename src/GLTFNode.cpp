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