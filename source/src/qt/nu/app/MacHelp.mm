#include "MacHelp.h"

#include <QFileInfo>

#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>

bool OpenDefcoinNuHelpBook(const QString& page)
{
    @autoreleasepool {
        NSURL* helpBookURL = [[NSBundle mainBundle] URLForResource:@"DefcoinCoreNu" withExtension:@"help"];
        if (helpBookURL) {
            AHRegisterHelpBookWithURL((__bridge CFURLRef)helpBookURL);
        }
        NSBundle* bundle = [NSBundle mainBundle];
        [[NSHelpManager sharedHelpManager] registerBooksInBundle:bundle];
        const QString clean_page = QFileInfo(page.trimmed().isEmpty() ? QStringLiteral("index.html") : page.trimmed()).fileName();
        NSString* anchor = clean_page == QStringLiteral("details.html") ? @"details" : @"index";
        NSString* path = clean_page == QStringLiteral("details.html") ? @"details.html" : @"index.html";
        NSString* bookTitle = @"Defcoin Core Nu Help";
        NSString* bookID = @"org.defcoincore.DefcoinCoreNu.help";

        OSStatus status = AHGotoPage((__bridge CFStringRef)bookTitle,
                                     (__bridge CFStringRef)path,
                                     (__bridge CFStringRef)anchor);
        if (status == noErr) {
            return true;
        }

        status = AHLookupAnchor((__bridge CFStringRef)bookTitle, (__bridge CFStringRef)anchor);
        if (status == noErr) {
            return true;
        }

        [[NSHelpManager sharedHelpManager] openHelpAnchor:anchor inBook:bookTitle];

        NSString* escapedAnchor = [anchor stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLQueryAllowedCharacterSet]];
        NSString* escapedBookID = [bookID stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLQueryAllowedCharacterSet]];
        NSString* urlString = [NSString stringWithFormat:@"help:anchor=%@ bookID=%@", escapedAnchor, escapedBookID];
        NSURL* helpURL = [NSURL URLWithString:urlString];
        if (helpURL && [[NSWorkspace sharedWorkspace] openURL:helpURL]) {
            return true;
        }
    }
    return false;
}
