#include "GLTFLoader.h"
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/Deserialize.h>
 //Replace this with <filesystem> (and use std::filesystem rather than
 //std::experimental::filesystem) if your toolchain fully supports C++17
#include <filesystem>

#include <sstream>

//using namespace Microsoft::glTF;
//
//// The glTF SDK is decoupled from all file I/O by the IStreamReader (and IStreamWriter)
//// interface(s) and the C++ stream-based I/O library. This allows the glTF SDK to be used in
//// sandboxed environments, such as WebAssembly modules and UWP apps, where any file I/O code
//// must be platform or use-case specific.
//class StreamReader : public IStreamReader
//{
//public:
//    StreamReader(std::filesystem::path pathBase) : m_pathBase(std::move(pathBase))
//    {
//        assert(m_pathBase.has_root_path());
//    }
//
//    // Resolves the relative URIs of any external resources declared in the glTF manifest
//    Shared<std::istream> GetInputStream(const String& filename) const override
//    {
//        // In order to construct a valid stream:
//        // 1. The filename argument will be encoded as UTF-8 so use filesystem::u8path to
//        //    correctly construct a path instance.
//        // 2. Generate an absolute path by concatenating m_pathBase with the specified filename
//        //    path. The filesystem::operator/ uses the platform's preferred directory separator
//        //    if appropriate.
//        // 3. Always open the file stream in binary mode. The glTF SDK will handle any text
//        //    encoding issues for us.
//        auto streamPath = m_pathBase / std::filesystem::u8path(filename);
//        auto stream = std::make_shared<std::ifstream>(streamPath, std::ios_base::binary);
//
//        // Check if the stream has no errors and is ready for I/O operations
//        if (!stream || !(*stream)) {
//            throw std::runtime_error("Unable to create a valid input stream for uri: " + filename);
//        }
//
//        return stream;
//    }
//
//private:
//    std::filesystem::path m_pathBase;
//};
//
//class GLTFDocument::Impl
//{
//public:
//    const Microsoft::glTF::Document& GetDocument() { return m_document; }
//    void SetDocument(Document&& document) { m_document = std::move(document); }
//private:
//    Document m_document;
//};
//
//GLTFDocument::GLTFDocument()
//    : m_pImpl(new GLTFDocument::Impl())
//{
//}
//
//
//GLTFDocument::~GLTFDocument()
//{
//    delete m_pImpl;
//    m_pImpl = nullptr;
//}
//
//const Microsoft::glTF::Document& GLTFDocument::GetDocument()
//{
//    return m_pImpl->GetDocument();
//}
//
//
//void GLTFDocument::SetDocument(Microsoft::glTF::Document&& document)
//{
//    m_pImpl->SetDocument(std::move(document));
//}
//
//RenderNode* GLTFLoader::Load(const String& name)
//{
//    return nullptr;
//    auto pGLTF = std::make_unique<GLTFDocument>();
//
//    std::filesystem::path dirPath = directory;
//    auto streamReader = std::make_unique<StreamReader>(dirPath);
//
//    auto gltfStream = streamReader->GetInputStream(fileName);
//    auto gltfResourceReader = std::make_unique<GLTFResourceReader>(std::move(streamReader));
//
//    std::stringstream manifestStream;
//
//    manifestStream << gltfStream->rdbuf();
//
//    Document document = Deserialize(manifestStream.str());
//
//    pGLTF->SetDocument(std::move(document));
//    Build(gltfResourceReader.get());
//}
