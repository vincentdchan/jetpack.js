//
// Created by Duzhong Chen on 2019/9/3.
//

#include "scanner.h"

using namespace std;

#define DO(EXPR) \
    if (!(EXPR)) return false;

Scanner::Scanner(std::shared_ptr<std::u32string> source): source_(std::move(source)) {

}

Scanner::ScannerState Scanner::SaveState() {
    ScannerState state {
        index_, line_number_, line_start_,
    };
    return state;
}

void Scanner::RestoreState(const Scanner::ScannerState &state) {
    index_ = state.index_;
    line_number_ = state.line_number_;
    line_start_ = state.line_start_;
}

void Scanner::UnexpectedToken() {
    CreateError("<Unexpected Token>", "", index_, line_number_, index_ - line_start_ + 1);
}

bool Scanner::SkipSingleLineComment(std::uint32_t offset, std::vector<Comment> &result) {
    std::uint32_t start = 0;
    SourceLocation loc;

    if (track_comment_) {
        start = index_ - offset;
        loc.start_.line_ = line_number_;
        loc.start_.column_ = index_ - line_start_ - offset;
    }

    while (!IsEnd()) {
        char32_t ch = (*source_)[index_];
        index_++;

        if (utils::IsLineTerminator(ch)) {
            if (track_comment_) {
                loc.end_ = Position { line_number_, index_ - line_start_ - 1 };
                Comment comment {
                    false,
                    make_pair(start + offset, index_ - 1),
                    make_pair(start, index_ - 1),
                    loc
                };
                result.push_back(comment);
            }
            if (ch == 13 && (*source_)[index_] == 10) {
                ++index_;
            }
            ++line_number_;
            line_start_ = index_;
            return true;
        }

    }

    if (track_comment_) {
        loc.end_ = Position { line_number_, index_ - line_start_ };
        Comment comment {
            false,
            make_pair(start + offset, index_),
            make_pair(start, index_),
            loc,
        };
        result.push_back(comment);
    }

    return true;
}

bool Scanner::SkipMultiLineComment(std::vector<Comment> &result) {
    std::uint32_t start = 0;
    SourceLocation loc;

    if (track_comment_) {
        start = index_ - 2;
        loc.start_ = Position {
            line_number_,
            index_ - line_start_ - 2,
        };
    }

    while (!IsEnd()) {
        char32_t ch = (*source_)[index_];
        if (utils::IsLineTerminator(ch)) {
            if (ch == 0x0D && (*source_)[index_ + 1] == 0x0A) {
                ++index_;
            }
            ++line_number_;
            ++index_;
            line_start_ = index_;
        } else if (ch == 0x2A) {
            if ((*source_)[index_ + 1] == 0x2F) {
                index_ += 2;
                if (track_comment_) {
                    loc.end_ = Position {
                        line_number_,
                        index_ - line_start_,
                    };
                    Comment comment {
                        true,
                        make_pair(start + 2, index_ -2),
                        make_pair(start, index_),
                        loc,
                    };
                    return true;
                }
            }

            ++index_;
        } else {
            ++index_;
        }
    }

    if (track_comment_) {
        loc.end_ = Position {
            line_number_,
            index_ - line_start_,
        };
        Comment comment {
            true,
            make_pair(start + 2, index_),
            make_pair(start, index_),
            loc,
        };
        result.push_back(comment);
    }

    this->UnexpectedToken();
    return true;
}

bool Scanner::ScanComments(std::vector<Comment> &result) {
    bool start = index_ == 0;

    while (!IsEnd()) {
        char32_t ch = (*source_)[index_];

        if (utils::IsWhiteSpace(ch)) {
            ++index_;
        } else if (utils::IsIdentifierStart(ch)) {
            ++index_;

            if (ch == 0x0D && (*source_)[index_ ] == 0x0A) {
                ++index_;
            }
            ++line_number_;
            line_start_ = index_;
            start = true;
        } else if (ch == 0x2F) {
            ch = (*source_)[index_ + 1];
            if (ch == 0x2F) {
                index_ += 2;
                vector<Comment> comments;
                DO(SkipSingleLineComment(2, comments));
                if (track_comment_) {
                    result.insert(result.end(), comments.begin(), comments.end());
                }
                start = true;
            } else if (ch == 0x2A) {  // U+002A is '*'
                index_ += 2;
                vector<Comment> comments;
                DO(SkipMultiLineComment(comments));
                if (track_comment_) {
                    result.insert(result.end(), comments.begin(), comments.end());
                }
            } else if (start && ch == 0x2D) { // U+002D is '-'
                // U+003E is '>'
                if (((*source_)[index_ + 1] == 0x2D) && ((*source_)[index_ + 2] == 0x3E)) {
                    // '-->' is a single-line comment
                    index_ += 3;
                    vector<Comment> comments;
                    DO(SkipSingleLineComment(3, comments))
                    if (track_comment_) {
                        result.insert(result.end(), comments.begin(), comments.end());
                    }
                } else {
                    break;
                }
            } else if (ch == 0x3C && !is_module_) { // U+003C is '<'
                if (source_->substr(index_ + 1, index_ + 4) == utils::To_UTF32("!--")) {
                    index_ += 4; // `<!--`
                    vector<Comment> comments;
                    DO(SkipSingleLineComment(4, comments))
                    if (track_comment_) {
                        result.insert(result.end(), comments.begin(), comments.end());
                    }
                } else {
                    break;
                }
            } else {
                break;
            }
        }

    }

    return true;
}
