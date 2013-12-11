/* ************************************************************************ */
/*                                                                          */
/* CMakePlugin for Codelite                                                 */
/* Copyright (C) 2013 Jiří Fatka <ntsfka@gmail.com>                         */
/*                                                                          */
/* This program is free software: you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or        */
/* (at your option) any later version.                                      */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.     */
/*                                                                          */
/* ************************************************************************ */

/* ************************************************************************ */
/* INCLUDES                                                                 */
/* ************************************************************************ */

// Declaration
#include "CMakeParser.h"

// C++
#include <cctype>

// wxWidgets
#include <wx/ffile.h>

// TODO REMOVE
#include <cstdlib>
#include <iostream>


/* ************************************************************************ */
/* STRUCTURES                                                               */
/* ************************************************************************ */

/**
 * @brief Parse token.
 */
struct Token
{
    /// Token start position.
    std::size_t start;

    /// Token length.
    std::size_t length;

    /// Token type.
    enum {
        TypeUnknown = 0,
        TypeIdentifier,
        TypeString,
        TypeLeftParen,
        TypeRightParen,
        TypeComment,
        TypeSpace,
        TypeVariable
    } type;

    /// String value.
    wxString value;
};

/* ************************************************************************ */

/**
 * @brief Iterator pair.
 */
struct IteratorPair
{
    /// Current position.
    wxString::const_iterator current;

    /// Input end position.
    wxString::const_iterator end;


    /**
     * @brief Returns current character.
     *
     * @return
     */
    wxUniChar Get() const {
        return *current;
    }


    /**
     * @brief Increment position iterator.
     *
     * @return this
     */
    IteratorPair& Next() {
        ++current;
        return *this;
    }


    /**
     * @brief Check if end of the input is reached.
     *
     * @return
     */
    bool IsEof() const {
        return current == end;
    }


    /**
     * @brief Test if current character match given character.
     *
     * @param ch Tested character.
     *
     * @return
     */
    bool Is(wxUniChar ch) const {
        return Get() == ch;
    }


    /**
     * @brief Test if current character is in given range.
     *
     * @param ch1
     * @param ch2
     *
     * @return
     */
    bool IsRange(wxUniChar ch1, wxUniChar ch2) const {
        return Get() >= ch1 && Get() <= ch2;
    }


    bool IsAlpha() const {
        return IsRange('a', 'z') || IsRange('A', 'Z');
    }


    bool IsAlphaNumberic() const {
        return IsAlpha() || IsRange('0', '9');
    }


    bool IsIdentifier() const {
        return IsAlphaNumberic() || Is('_');
    }
};

/* ************************************************************************ */
/* FUNCTIONS                                                                */
/* ************************************************************************ */

/**
 * @brief Parses a token from input stream.
 *
 * @param context Parsing context.
 * @param token   Output token.
 *
 * @return If some token was parsed.
 */
static bool GetToken(IteratorPair& context, Token& token)
{
    // EOS
    if (context.IsEof())
        return false;

    token.type = Token::TypeUnknown;
    token.value.clear();

    if (context.Is('#')) {

        // COMMENT
        token.type = Token::TypeComment;

        // Everything until EOL is part of the comment
        while (!context.IsEof() && !context.Is('\n')) {
            token.value += context.Get();
            context.Next();
        }

        context.Next();

    } else if (context.Is('(')) {

        // LEFT PARENTHESIS
        token.type = Token::TypeLeftParen;
        token.value += context.Get();
        context.Next();

    } else if (context.Is(')')) {

        // RIGHT PARENTHESIS
        token.type = Token::TypeRightParen;
        token.value += context.Get();
        context.Next();

    } else if (context.Is(' ') || context.Is('\n') || context.Is('\t')) {

        // WHITESPACE
        token.type = Token::TypeSpace;
        token.value += context.Get();
        context.Next();

    } else if (context.IsAlpha()) {

        token.type = Token::TypeIdentifier;
        token.value += context.Get();
        context.Next();

        // Rest of the identifier
        while (!context.IsEof() && context.IsIdentifier()) {
            token.value += context.Get();
            context.Next();
        }

    } else if (context.Is('$')) {

        // VARIABLE
        token.value += context.Get();
        context.Next();

        if (context.Is('{'))
        {
            token.type = Token::TypeVariable;
            token.value += context.Get();
            context.Next();

            // Rest of the identifier
            while (!context.IsEof() && context.IsIdentifier()) {
                token.value += context.Get();
                context.Next();
            }

            // TODO Check }
            token.value += context.Get();
            context.Next();
        }

    } else {

        token.value += context.Get();
        context.Next();

    }

    return !context.IsEof();
}

/* ************************************************************************ */
/* CLASSES                                                                  */
/* ************************************************************************ */

CMakeParser::CMakeParser()
{
    // Nothing to do
}

/* ************************************************************************ */

void
CMakeParser::Clear()
{
    m_filename.Clear();
    m_commands.clear();
}

/* ************************************************************************ */

bool
CMakeParser::Parse(const wxString& content)
{
    Token token;
    IteratorPair context = {content.begin(), content.end() };

    // Parse input into tokens
    while (GetToken(context, token)) {
        std::cout << token.type << ": " << token.value << std::endl;
    }
}

/* ************************************************************************ */

bool
CMakeParser::ParseFile(const wxFileName& filename)
{
    // Clear everything
    Clear();

    m_filename = filename;

    // Open file
    wxFFile file(m_filename.GetFullPath());

    // File cannot be opened
    if (!file.IsOpened())
        return false;

    // Read file content
    wxString content;
    file.ReadAll(&content);

    return Parse(content);
}

/* ************************************************************************ */

#ifdef MAIN

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Missing parameter" << std::endl;
        return EXIT_FAILURE;
    }

    const wxFileName filename(argv[1]);

    CMakeParser parser;
    if (!parser.ParseFile(filename)) {
        std::cerr << "Unable to open file" << std::endl;
        return EXIT_FAILURE;
    }

    const wxVector<CMakeParser::Command>& cmds = parser.GetCommands();

    for (wxVector<CMakeParser::Command>::const_iterator it = cmds.begin(), ite = cmds.end(); it != ite; ++it) {
        std::cout << it->name << "(" << ")" << std::endl;
    }

    return EXIT_SUCCESS;
}

#endif

/* ************************************************************************ */
