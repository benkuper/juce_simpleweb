/*
  ==============================================================================

    juce_SimpleWebSocket.h
    Created: 17 Jun 2020 11:22:54pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MIMETypes
{
public:

    static bool isInit;
    static juce::HashMap<juce::String, juce::String> mimeTypes;

    static juce::String getMIMEType(const juce::String& extension)
    {
        if (!isInit)
        {
            init();
        }
        return mimeTypes.contains(extension) ? mimeTypes[extension] : "";
    }

protected:
    static void init()
    {
        mimeTypes.set(".aac", "audio/aac");
        mimeTypes.set(".abw", "application/x-abiword");
        mimeTypes.set(".arc", "application/octet-stream");
        mimeTypes.set(".avi", "video/x-msvideo");
        mimeTypes.set(".azw", "application/vnd.amazon.ebook");
        mimeTypes.set(".bin", "application/octet-stream");
        mimeTypes.set(".bmp", "image/bmp");
        mimeTypes.set(".bz", "application/x-bzip");
        mimeTypes.set(".bz2", "application/x-bzip2");
        mimeTypes.set(".csh", "application/x-csh");
        mimeTypes.set(".css", "text/css");
        mimeTypes.set(".csv", "text/csv");
        mimeTypes.set(".doc", "application/msword");
        mimeTypes.set(".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
        mimeTypes.set(".eot", "application/vnd.ms-fontobject");
        mimeTypes.set(".epub", "application/epub+zip");
        mimeTypes.set(".gif", "image/gif");
        mimeTypes.set(".html", "text/html");
        mimeTypes.set(".html", "text/html");
        mimeTypes.set(".ico", "image/x-icon");
        mimeTypes.set(".ics", "text/calendar");
        mimeTypes.set(".jar", "application/java-archive");
        mimeTypes.set(".jpeg", "image/jpeg");
        mimeTypes.set(".jpg", "image/jpeg");
        mimeTypes.set(".js", "application/javascript");
        mimeTypes.set(".json", "application/json");
        mimeTypes.set(".mid", "audio/midi");
        mimeTypes.set(".midi", "audio/midi");
        mimeTypes.set(".mpeg", "video/mpeg");
        mimeTypes.set(".mp4", "video/mp4s");
        mimeTypes.set(".mpkg", "application/vnd.apple.installer+xml");
        mimeTypes.set(".odp", "application/vnd.oasis.opendocument.presentation");
        mimeTypes.set(".ods", "application/vnd.oasis.opendocument.spreadsheet");
        mimeTypes.set(".odt", "application/vnd.oasis.opendocument.text");
        mimeTypes.set(".oga", "audio/ogg");
        mimeTypes.set(".ogv", "video/ogg");
        mimeTypes.set(".ogx", "application/ogg");
        mimeTypes.set(".otf", "font/otf");
        mimeTypes.set(".png", "image/png");
        mimeTypes.set(".pdf", "application/pdf");
        mimeTypes.set(".ppt", "application/vnd.ms-powerpoint");
        mimeTypes.set(".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation");
        mimeTypes.set(".rar", "application/x-rar-compressed");
        mimeTypes.set(".rtf", "application/rtf");
        mimeTypes.set(".sh", "application/x-sh");
        mimeTypes.set(".svg", "image/svg+xml");
        mimeTypes.set(".swf", "application/x-shockwave-flash");
        mimeTypes.set(".tar", "application/x-tar");
        mimeTypes.set(".tif", "image/tiff");
        mimeTypes.set(".tiff", "image/tiff");
        mimeTypes.set(".ts", "application/typescript");
        mimeTypes.set(".ttf", "font/ttf");
        mimeTypes.set(".vsd", "application/vnd.visio");
        mimeTypes.set(".wav", "audio/x-wav");
        mimeTypes.set(".weba", "audio/webm");
        mimeTypes.set(".webm", "video/webm");
        mimeTypes.set(".webp", "image/webp");
        mimeTypes.set(".woff", "font/woff");
        mimeTypes.set(".woff2", "font/woff2");
        mimeTypes.set(".xhtml", "application/xhtml+xml");
        mimeTypes.set(".xls", "application/vnd.ms-excel");
        mimeTypes.set(".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
        mimeTypes.set(".xml", "application/xml");
        mimeTypes.set(".xul", "application/vnd.mozilla.xul+xml");
        mimeTypes.set(".zip", "application/zip");
        mimeTypes.set(".3gp", "video/3gpp");
        mimeTypes.set(".3g2", "video/3gpp2");
        mimeTypes.set(".7z", "application/x-7z-compressed");


        isInit = true;
    }
};

