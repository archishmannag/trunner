#include "llmclient.hpp"

namespace llm
{

    c_client::c_client(s_config config) : m_config(std::move(config))
    {
        m_networkManager = std::make_unique<QNetworkAccessManager>();
    }

    auto c_client::send_message(const QString &prompt) -> std::expected<QString, s_error>
    {

        auto request = build_request();
        auto payload = build_payload(prompt);

        QEventLoop loop;
        QNetworkReply *reply = m_networkManager->post(request, payload);

        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

        QTimer::singleShot(m_config.timeout_ms, &loop, [&loop, reply]()
                           {
                           reply->abort();
                           loop.quit(); });

        loop.exec();

        if (reply->error() == QNetworkReply::TimeoutError)
        {
            reply->deleteLater();
            return std::unexpected(s_error{ .code = e_error_code::timeout, .message = QStringLiteral("Request timed out") });
        }

        if (reply->error() != QNetworkReply::NoError)
        {
            auto error_message = reply->errorString();
            reply->deleteLater();
            return std::unexpected(s_error{ .code = e_error_code::network_error, .message = error_message });
        }

        auto response_data = reply->readAll();
        reply->deleteLater();

        return parse_response(response_data);
    }

    auto c_client::build_request() const -> QNetworkRequest
    {
        QNetworkRequest request;
        request.setUrl(QUrl(get_endpoint()));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

        switch (m_config.provider)
        {
        case e_provider::OpenAI:
        case e_provider::OpenRouter:
        case e_provider::Groq:
            request.setRawHeader("Authorization",
                                 QStringLiteral("Bearer %1").arg(m_config.apiKey).toUtf8());
            break;
        case e_provider::Anthropic:
            request.setRawHeader("x-api-key", m_config.apiKey.toUtf8());
            request.setRawHeader("anthropic-version", "2023-06-01");
            break;
        case e_provider::Gemini:
            request.setRawHeader("x-goog-api-key", m_config.apiKey.toUtf8());
            break;
        }

        return request;
    }

    auto c_client::build_payload(const QString &prompt_text) const -> QByteArray
    {
        QJsonObject json;

        QString prompt = prompt_text + QStringLiteral("\nAnswer in few lines. Preferably 2-3 sentences.");

        switch (m_config.provider)
        {
        case e_provider::OpenAI:
        case e_provider::OpenRouter:
        case e_provider::Groq:
        {
            QJsonArray messages;
            QJsonObject message;
            message[QStringLiteral("role")] = QStringLiteral("user");
            message[QStringLiteral("content")] = prompt;
            messages.append(message);

            json[QStringLiteral("model")] = m_config.model;
            json[QStringLiteral("messages")] = messages;
            json[QStringLiteral("max_tokens")] = m_config.max_tokens;
            break;
        }
        case e_provider::Anthropic:
        {
            QJsonArray messages;
            QJsonObject message;
            message[QStringLiteral("role")] = QStringLiteral("user");
            message[QStringLiteral("content")] = prompt;
            messages.append(message);

            json[QStringLiteral("model")] = m_config.model;
            json[QStringLiteral("messages")] = messages;
            json[QStringLiteral("max_tokens")] = m_config.max_tokens;
            break;
        }
        case e_provider::Gemini:
        {
            QJsonArray parts;
            QJsonObject part;
            part[QStringLiteral("text")] = prompt;
            parts.append(part);

            QJsonArray contents;
            QJsonObject content;
            content[QStringLiteral("parts")] = parts;
            contents.append(content);

            json[QStringLiteral("contents")] = contents;

            QJsonObject generation_config;
            generation_config[QStringLiteral("maxOutputTokens")] = m_config.max_tokens;
            json[QStringLiteral("generationConfig")] = generation_config;
            break;
        }
        }

        return QJsonDocument(json).toJson(QJsonDocument::Compact);
    }

    auto c_client::parse_response(const QByteArray &data) const -> std::expected<QString, s_error>
    {

        auto doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject())
        {
            return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = QStringLiteral("Invalid JSON response") });
        }

        auto obj = doc.object();

        // Check for error response
        if (obj.contains(QStringLiteral("error")))
        {
            auto error_obj = obj[QStringLiteral("error")].toObject();
            auto error_msg = error_obj[QStringLiteral("message")].toString();
            return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = error_msg });
        }

        QString content;

        switch (m_config.provider)
        {
        case e_provider::OpenAI:
        case e_provider::OpenRouter:
        case e_provider::Groq:
        {
            if (!obj.contains(QStringLiteral("choices")))
            {
                return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = QStringLiteral("Missing 'choices' in response") });
            }

            auto choices = obj[QStringLiteral("choices")].toArray();
            if (choices.isEmpty())
            {
                return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = QStringLiteral("Empty choices array") });
            }

            auto first_choice = choices[0].toObject();
            auto message = first_choice[QStringLiteral("message")].toObject();
            content = message[QStringLiteral("content")].toString();
            break;
        }
        case e_provider::Anthropic:
        {
            if (!obj.contains(QStringLiteral("content")))
            {
                return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = QStringLiteral("Missing 'content' in response") });
            }

            auto content_array = obj[QStringLiteral("content")].toArray();
            if (content_array.isEmpty())
            {
                return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = QStringLiteral("Empty content array") });
            }

            auto first_content = content_array[0].toObject();
            content = first_content[QStringLiteral("text")].toString();
            break;
        }
        case e_provider::Gemini:
        {
            if (!obj.contains(QStringLiteral("candidates")))
            {
                return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = QStringLiteral("Missing 'candidates' in response") });
            }

            auto candidates = obj[QStringLiteral("candidates")].toArray();
            if (candidates.isEmpty())
            {
                return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = QStringLiteral("Empty candidates array") });
            }

            auto first_candidate = candidates[0].toObject();
            auto content_obj = first_candidate[QStringLiteral("content")].toObject();
            auto parts = content_obj[QStringLiteral("parts")].toArray();

            if (parts.isEmpty())
            {
                return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = QStringLiteral("Empty parts array") });
            }

            auto first_part = parts[0].toObject();
            content = first_part[QStringLiteral("text")].toString();
            break;
        }
        }

        if (content.isEmpty())
        {
            return std::unexpected(s_error{ .code = e_error_code::invalid_response, .message = QStringLiteral("Empty response content") });
        }

        return content;
    }

    auto c_client::get_endpoint() const -> QString
    {
        switch (m_config.provider)
        {
        case e_provider::OpenAI:
            return QStringLiteral("https://api.openai.com/v1/chat/completions");
        case e_provider::Anthropic:
            return QStringLiteral("https://api.anthropic.com/v1/messages");
        case e_provider::OpenRouter:
            return QStringLiteral("https://openrouter.ai/api/v1/chat/completions");
        case e_provider::Gemini:
            return QStringLiteral("https://generativelanguage.googleapis.com/v1beta/models/%1:generateContent")
                .arg(m_config.model);
        case e_provider::Groq:
            return QStringLiteral("https://api.groq.com/openai/v1/chat/completions");
        }
        return {};
    }

} // namespace llm
