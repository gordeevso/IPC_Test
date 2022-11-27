add_library(Common SHARED Common.cpp)
target_include_directories(Common PUBLIC include/Common ../3rdParty/readerwriterqueue)
target_link_libraries(Common PRIVATE readerwriterqueue)

add_library(EntriesProcessing SHARED EntriesProcessing.cpp)
target_include_directories(EntriesProcessing PUBLIC include/EntriesProcessing)
target_link_libraries(EntriesProcessing PRIVATE Common)

add_executable(ComponentA ComponentA.cpp)
target_include_directories(ComponentA PRIVATE include)
target_link_libraries(ComponentA PRIVATE Common EntriesProcessing)

add_executable(ComponentB ComponentB.cpp)
target_include_directories(ComponentB PRIVATE include)
target_link_libraries(ComponentB PRIVATE Common EntriesProcessing)
