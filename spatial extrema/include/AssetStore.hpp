#ifndef ASSETSTORE_HPP
#define ASSETSTORE_HPP

#include <memory>
#include <unordered_map>
#include <string>

#include <ngl/Obj.h>
#include <ngl/Singleton.h>
#include <SDL.h>

//The root folder for asset storage.
const std::string g_resourceLocation = "resources/";

class AssetStore
{
public:
	AssetStore() = default;
	~AssetStore();

	ngl::Obj * getModel(const std::string &_id);
	GLuint getTexture(const std::string &_id);

	//Loads a mesh from _path, stores it with  _id.
	void loadMesh(const std::string &_id, const std::string &_path);
	//Loads a texture from _path, stores it with _id.
	void loadTexture(const std::string &_id, const std::string &_path);
private:
	AssetStore(const AssetStore &_store) = delete;

	//Internal method to help me load in images.
	GLuint SDLSurfaceToGLTexture( SDL_Surface * _surf );

	std::unordered_map< std::string, std::unique_ptr<ngl::Obj> > m_meshes;
	std::unordered_map< std::string, GLuint > m_textures;
};

#endif
