#include <boost/filesystem.hpp>
#include <wx/image.h>

void dcm2img(boost::filesystem::path filename, int width, int height, wxImage &image);
void RegisterCodecs();
void DeregisterCodecs();