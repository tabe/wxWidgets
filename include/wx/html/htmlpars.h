/////////////////////////////////////////////////////////////////////////////
// Name:        htmlpars.h
// Purpose:     wxHtmlParser class (generic parser)
// Author:      Vaclav Slavik
// RCS-ID:      $Id$
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////


#ifndef _WX_HTMLPARS_H_
#define _WX_HTMLPARS_H_

#ifdef __GNUG__
#pragma interface "htmlpars.h"
#endif

#include "wx/defs.h"
#if wxUSE_HTML

#include "wx/html/htmltag.h"
#include "wx/filesys.h"

class wxHtmlParser;
class wxHtmlTagHandler;

//--------------------------------------------------------------------------------
// wxHtmlParser
//                  This class handles generic parsing of HTML document : it scans
//                  the document and divide it into blocks of tags (where one block
//                  consists of starting and ending tag and of text between these
//                  2 tags.
//--------------------------------------------------------------------------------

class WXDLLEXPORT wxHtmlParser : public wxObject
{
    DECLARE_ABSTRACT_CLASS(wxHtmlParser)

public:
    wxHtmlParser() : wxObject(), m_HandlersHash(wxKEY_STRING) 
        { m_FS = NULL; m_Cache = NULL; m_HandlersStack = NULL; }
    virtual ~wxHtmlParser();

    // Sets the class which will be used for opening files
    void SetFS(wxFileSystem *fs) { m_FS = fs; }

    wxFileSystem* GetFS() const { return m_FS; }

    // You can simply call this method when you need parsed output.
    // This method does these things:
    // 1. call InitParser(source);
    // 2. call DoParsing();
    // 3. call GetProduct(); (it's return value is then returned)
    // 4. call DoneParser();
    wxObject* Parse(const wxString& source);

    // Sets the source. This must be called before running Parse() method.
    virtual void InitParser(const wxString& source);
    // This must be called after Parse().
    virtual void DoneParser();

    // Parses the m_Source from begin_pos to end_pos-1.
    // (in noparams version it parses whole m_Source)
    void DoParsing(int begin_pos, int end_pos);
    inline void DoParsing() {DoParsing(0, m_Source.Length());};

    // Returns product of parsing
    // Returned value is result of parsing of the part. The type of this result
    // depends on internal representation in derived parser
    // (see wxHtmlWinParser for details).
    virtual wxObject* GetProduct() = 0;

    // adds handler to the list & hash table of handlers.
    virtual void AddTagHandler(wxHtmlTagHandler *handler);

    // Forces the handler to handle additional tags (not returned by GetSupportedTags). 
    // The handler should already be in use by this parser.
    // Example: you want to parse following pseudo-html structure:
    //   <myitems>
    //     <it name="one" value="1">
    //     <it name="two" value="2">
    //   </myitems>
    //   <it> This last it has different meaning, we don't want it to be parsed by myitems handler!
    // handler can handle only 'myitems' (e.g. it's GetSupportedTags returns "MYITEMS")
    // you can call PushTagHandler(handler, "IT") when you find <myitems>
    // and call PopTagHandler() when you find </myitems>
    void PushTagHandler(wxHtmlTagHandler *handler, wxString tags);

    // Restores state before last call to PushTagHandler
    void PopTagHandler();

    wxString* GetSource() {return &m_Source;}
    void SetSource(const wxString& src);

protected:
    // Adds text to the output.
    // This is called from Parse() and must be overriden in derived classes.
    // txt is not guaranteed to be only one word. It is largest continuous part of text
    // (= not broken by tags)
    // NOTE : using char* because of speed improvements
    virtual void AddText(const char* txt) = 0;

    // Adds tag and proceeds it. Parse() may (and usually is) called from this method.
    // This is called from Parse() and may be overriden.
    // Default behavior is that it looks for proper handler in m_Handlers. The tag is
    // ignored if no hander is found.
    // Derived class is *responsible* for filling in m_Handlers table.
    virtual void AddTag(const wxHtmlTag& tag);

protected:
    // source being parsed
    wxString m_Source;

    // tags cache, used during parsing.
    wxHtmlTagsCache *m_Cache;
    wxHashTable m_HandlersHash;

    // handlers that handle particular tags. The table is accessed by
    // key = tag's name.
    // This attribute MUST be filled by derived class otherwise it would
    // be empty and no tags would be recognized
    // (see wxHtmlWinParser for details about filling it)
    // m_HandlersHash is for random access based on knowledge of tag name (BR, P, etc.)
    //      it may (and often does) contain more references to one object
    // m_HandlersList is list of all handlers and it is guaranteed to contain
    //      only one reference to each handler instance.
    wxList m_HandlersList;

    // class for opening files (file system)
    wxFileSystem *m_FS;
    // handlers stack used by PushTagHandler and PopTagHandler
    wxList *m_HandlersStack;
};






//--------------------------------------------------------------------------------
// wxHtmlTagHandler
//                  This class (and derived classes) cooperates with wxHtmlParser.
//                  Each recognized tag is passed to handler which is capable
//                  of handling it. Each tag is handled in 3 steps:
//                  1. Handler will modifies state of parser
//                    (using it's public methods)
//                  2. Parser parses source between starting and ending tag
//                  3. Handler restores original state of the parser
//--------------------------------------------------------------------------------

class WXDLLEXPORT wxHtmlTagHandler : public wxObject
{
    DECLARE_ABSTRACT_CLASS(wxHtmlTagHandler)

public:
    wxHtmlTagHandler() : wxObject () { m_Parser = NULL; }

    // Sets the parser.
    // NOTE : each _instance_ of handler is guaranteed to be called
    // only by one parser. This means you don't have to care about
    // reentrancy.
    virtual void SetParser(wxHtmlParser *parser) 
        { m_Parser = parser; }

    // Returns list of supported tags. The list is in uppercase and
    // tags are delimited by ','.
    // Example : "I,B,FONT,P"
    //   is capable of handling italic, bold, font and paragraph tags
    virtual wxString GetSupportedTags() = 0;

    // This is hadling core method. It does all the Steps 1-3.
    // To process step 2, you can call ParseInner()
    // returned value : TRUE if it called ParseInner(),
    //                  FALSE etherwise
    virtual bool HandleTag(const wxHtmlTag& tag) = 0;

protected:
    // parses input between beginning and ending tag.
    // m_Parser must be set.
    void ParseInner(const wxHtmlTag& tag) 
        { m_Parser->DoParsing(tag.GetBeginPos(), tag.GetEndPos1()); }

    wxHtmlParser *m_Parser;
};




#endif

#endif // _WX_HTMLPARS_H_
