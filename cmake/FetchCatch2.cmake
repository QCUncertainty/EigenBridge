include(FetchContent)

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG "v3.11.0"
)

FetchContent_MakeAvailable(catch2)
