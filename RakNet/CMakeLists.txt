project(RakHook)
cmake_minimum_required(VERSION 3.10)

add_library(RakNet
    RakNet/BitStream.cpp
    RakNet/BitStream.h
    RakNet/DS_HuffmanEncodingTree.cpp
    RakNet/DS_HuffmanEncodingTree.h
    RakNet/DS_HuffmanEncodingTreeNode.h
    RakNet/DS_LinkedList.h
    RakNet/DS_List.h
    RakNet/DS_Map.h
    RakNet/DS_OrderedList.h
    RakNet/DS_Queue.h
    RakNet/NetworkTypes.h
    RakNet/PacketEnumerations.h
    RakNet/PacketPriority.h
    RakNet/PluginInterface.cpp
    RakNet/PluginInterface.h
    RakNet/RakClientInterface.h
    RakNet/StringCompressor.cpp
    RakNet/StringCompressor.h
)
target_include_directories(RakNet PUBLIC .)
if (NOT ${MSVC})
    target_compile_options(RakNet PRIVATE -fpermissive)
endif()