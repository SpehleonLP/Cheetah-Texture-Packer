#ifndef GLBLITVAO_H
#define GLBLITVAO_H
#include <cstdint>

class GLViewWidget;

namespace glDefaultVAOs
{
    void AddRef();
    void Release(GLViewWidget * gl);

    void BindVAO(GLViewWidget * gl);

    void RenderPoint(GLViewWidget * gl);
    void RenderSquare(GLViewWidget * gl);
    void RenderCube(GLViewWidget * gl);

	void BindSquareIndexVBO(GLViewWidget * gl);

	uint32_t GetWhiteTexture(GLViewWidget * gl);
	uint32_t GetNormalTexture(GLViewWidget * gl);
}


#endif // GLBLITVAO_H
