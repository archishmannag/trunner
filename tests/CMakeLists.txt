add_executable(test_llmclient test_llmclient.cpp)
target_link_libraries(test_llmclient
    PRIVATE
    Qt6::Test
    Qt6::Network
    llmclient
)

add_test(NAME test_llmclient COMMAND test_llmclient)

# Mock test for the runner (requires actual KRunner setup)
add_executable(test_llmrunner test_llmrunner.cpp)
target_sources(test_llmrunner
    PRIVATE
    ../src/llmrunner.cpp
    ../src/llmrunner.hpp
)
target_link_libraries(test_llmrunner
    PRIVATE
    Qt6::Test
    Qt6::Gui
    KF6::Runner
    KF6::I18n
    KF6::ConfigCore
    llmclient
)

add_test(NAME test_llmrunner COMMAND test_llmrunner)
