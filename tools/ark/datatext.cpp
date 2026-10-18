#include "datatext.h"

#include <cctype>
#include <charconv>
#include <format>
#include <string>
#include <utility>
#include <vector>

#include "imageerror.h"

namespace Tools::Ark {

using Common::Error;
using Common::invalidInput;

namespace {

std::unexpected<Error> invalid(std::size_t offset, std::string_view what) {
    return invalidInput(std::format("{} at offset {}.", what, offset));
}

bool isDelimiter(char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0 || c == '(' || c == ')' || c == '[' ||
           c == ']' || c == '{' || c == '}' || c == '\'' || c == '"' || c == ';';
}

class Parser {
public:
    explicit Parser(std::string_view text) : text_(text) {
    }

    std::expected<DataFile, Error> run() {
        DataFile file;
        file.root = DataNode::makeArray(0, 0);
        skip();
        if (peek('{')) {
            ++offset_;
            for (skip(); !peek('}'); skip()) {
                auto name = quoted('"');
                if (!name) {
                    return std::unexpected(name.error());
                }
                file.files.push_back(std::move(*name));
            }
            ++offset_;
        }
        std::vector<DataNode *> stack{&file.root};
        for (skip(); offset_ < text_.size(); skip()) {
            const auto c = text_[offset_];
            if (c == '(') {
                ++offset_;
                auto &children = stack.back()->children;
                children.push_back(DataNode::makeArray(0, 0));
                stack.push_back(&children.back());
            } else if (c == ')') {
                if (stack.size() == 1) {
                    return invalid(offset_, "An unmatched closing parenthesis");
                }
                ++offset_;
                stack.pop_back();
            } else if (c == '[') {
                if (auto read = location(*stack.back()); !read) {
                    return std::unexpected(read.error());
                }
            } else if (c == '\'' || c == '"') {
                auto symbol = quoted(c);
                if (!symbol) {
                    return std::unexpected(symbol.error());
                }
                stack.back()->children.push_back(DataNode::makeSymbol(std::move(*symbol)));
            } else {
                auto element = word();
                if (!element) {
                    return std::unexpected(element.error());
                }
                stack.back()->children.push_back(std::move(*element));
            }
        }
        if (stack.size() != 1) {
            return invalid(offset_, "An unclosed array");
        }
        return file;
    }

private:
    bool peek(char c) const {
        return offset_ < text_.size() && text_[offset_] == c;
    }

    void skip() {
        while (offset_ < text_.size()) {
            if (text_[offset_] == ';') {
                while (offset_ < text_.size() && text_[offset_] != '\n') {
                    ++offset_;
                }
            } else if (std::isspace(static_cast<unsigned char>(text_[offset_])) != 0) {
                ++offset_;
            } else {
                return;
            }
        }
    }

    std::expected<std::string, Error> quoted(char quote) {
        if (!peek(quote)) {
            return invalid(offset_, "A missing quotation mark");
        }
        const auto start = ++offset_;
        const auto end = text_.find(quote, start);
        if (end == std::string_view::npos) {
            return invalid(start, "An unterminated quotation");
        }
        offset_ = end + 1;
        return std::string(text_.substr(start, end - start));
    }

    // The line may be blank, as in `[ 0]`, and is then zero.
    std::expected<void, Error> location(DataNode &node) {
        const auto start = ++offset_;
        const auto end = text_.find(']', start);
        if (end == std::string_view::npos) {
            return invalid(start, "An unterminated location");
        }
        offset_ = end + 1;
        std::vector<long> numbers;
        for (auto at = start; at < end;) {
            while (at < end && text_[at] == ' ') {
                ++at;
            }
            if (at == end) {
                break;
            }
            long number = 0;
            const auto [last, error] =
                std::from_chars(text_.data() + at, text_.data() + end, number);
            if (error != std::errc()) {
                return invalid(at, "A location that is not a number");
            }
            numbers.push_back(number);
            at = static_cast<std::size_t>(last - text_.data());
        }
        if (numbers.empty() || numbers.size() > 2) {
            return invalid(start, "A location without one or two numbers");
        }
        node.line = numbers.size() == 2 ? static_cast<std::uint16_t>(numbers.front()) : 0;
        node.file = static_cast<std::int32_t>(numbers.back());
        return {};
    }

    std::expected<DataNode, Error> word() {
        const auto start = offset_;
        while (offset_ < text_.size() && !isDelimiter(text_[offset_])) {
            ++offset_;
        }
        const auto token = text_.substr(start, offset_ - start);
        if (token.empty()) {
            return invalid(start, "An unexpected character");
        }
        const auto *first = token.data();
        const auto *last = token.data() + token.size();
        std::int32_t integer = 0;
        if (const auto [end, error] = std::from_chars(first, last, integer);
            error == std::errc() && end == last) {
            return DataNode::makeInt(integer);
        }
        float real = 0;
        if (const auto [end, error] = std::from_chars(first, last, real);
            error == std::errc() && end == last) {
            return DataNode::makeFloat(real);
        }
        return DataNode::makeSymbol(std::string(token));
    }

    std::string_view text_;
    std::size_t offset_ = 0;
};

} // namespace

std::expected<DataFile, Error> DataText::parse(std::string_view text) {
    return Parser(text).run();
}

} // namespace Tools::Ark
