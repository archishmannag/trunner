# LLM Client Library
add_library(llmclient STATIC)
target_sources(llmclient
    PRIVATE
    llmclient.cpp
    llmclient.hpp
)
target_include_directories(llmclient
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(llmclient
    PUBLIC
    Qt6::Core
    Qt6::Network
)
set_target_properties(llmclient PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Runner Plugin
add_library(krunner_llm MODULE)

target_sources(krunner_llm
    PRIVATE
    llmrunner.cpp
    llmrunner.hpp
    plasma-runner-llm.json
)

target_link_libraries(krunner_llm
    PRIVATE
    KF6::Runner
    KF6::I18n
    KF6::ConfigCore
    KF6::ConfigWidgets
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    Qt6::Gui
    llmclient
)

# Install plugin
install(TARGETS krunner_llm DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/kf6/krunner)

# Install metadata
install(FILES plasma-runner-llm.desktop DESTINATION ${KDE_INSTALL_DATADIR}/krunner/dbusplugins)

# Configuration UI
add_library(kcm_krunner_llm MODULE)
set_target_properties(kcm_krunner_llm PROPERTIES AUTOUIC ON)
target_sources(kcm_krunner_llm
    PRIVATE
    llmconfig.cpp
    llmconfig.hpp
    kcm_krunner_llm.json
)

ki18n_wrap_ui(kcm_krunner_llm llmconfig.ui)

target_link_libraries(kcm_krunner_llm
    PRIVATE
    KF6::ConfigCore
    KF6::ConfigWidgets
    KF6::KCMUtils
    KF6::I18n
    Qt6::Widgets
)

install(TARGETS kcm_krunner_llm DESTINATION ${KDE_INSTALL_QTPLUGINDIR})
install(FILES kcm_krunner_llm.desktop DESTINATION ${KDE_INSTALL_DATADIR}/krunner/kcms)
