#ifndef LLMMODULE_H
#define LLMMODULE_H

#include <KCModule>
#include <QWidget>

namespace Ui
{
    class LLMConfigWidget;
} // namespace ui

class c_llm_config : public KCModule
{
    Q_OBJECT

public:
    explicit c_llm_config(QObject *parent, const KPluginMetaData &metaData);
    ~c_llm_config() override;

    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void on_provider_changed(int index);
    void on_settings_changed();

private:
    Ui::LLMConfigWidget *m_ui;
    QString m_triggerWord;
};

#endif // LLMMODULE_H
