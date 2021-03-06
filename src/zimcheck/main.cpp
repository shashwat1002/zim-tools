/*
 * Copyright (C) 2006 Tommi Maekitalo
 * Copyright (C) Kiran Mathew Koshy
 * Copyright (C) Matthieu Gautier <mgautier@kymeria.fr>
 * Copyright (C) Emmanuel Engelhart <kelson@kiwix.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <unistd.h>
#include <zim/archive.h>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <regex>
#include <ctime>
#include <unordered_map>

#include "../progress.h"
#include "../version.h"
#include "../tools.h"
#include "checks.h"

void displayHelp()
{
    std::cout<<"\n"
             "zimcheck checks the quality of a ZIM file.\n\n"
             "Usage: zimcheck [options] zimfile\n"
             "options:\n"
             "-A , --all             run all tests. Default if no flags are given.\n"
             "-0 , --empty           Empty content\n"
             "-C , --checksum        Internal CheckSum Test\n"
             "-I , --integrity       Low-level correctness/integrity checks\n"
             "-M , --metadata        MetaData Entries\n"
             "-F , --favicon         Favicon\n"
             "-P , --main            Main page\n"
             "-R , --redundant       Redundant data check\n"
             "-U , --url_internal    URL check - Internal URLs\n"
             "-X , --url_external    URL check - External URLs\n"
             "-E , --mime            MIME checks\n"
             "-D , --details         Details of error\n"
             "-B , --progress        Print progress report\n"
             "-H , --help            Displays Help\n"
             "-V , --version         Displays software version\n"
             "examples:\n"
             "zimcheck -A wikipedia.zim\n"
             "zimcheck --checksum --redundant wikipedia.zim\n"
             "zimcheck -F -R wikipedia.zim\n"
             "zimcheck -M --favicon wikipedia.zim\n";
    return;
}

int main (int argc, char **argv)
{
    // To calculate the total time taken by the program to run.
    time_t startTime,endTime;
    double  timeDiffference;
    time( &startTime);

    // The boolean values which will be used to store the output from
    // getopt_long().  These boolean values will be then read by the
    // program to execute the different parts of the program.

    bool run_all = false;
    bool checksum = false;
    bool metadata = false;
    bool favicon = false;
    bool main_page = false;
    bool redundant_data = false;
    bool integrity = false;
    bool url_check = false;
    bool url_check_external = false;
    bool empty_check = false;
    bool mime_check = false;
    bool error_details = false;
    bool no_args = true;
    bool help = false;

    std::string filename = "";
    ProgressBar progress(1);
    ErrorLogger error;

    StatusCode status_code = PASS;

    //Parsing through arguments using getopt_long(). Both long and short arguments are allowed.
    while (1)
    {
        static struct option long_options[] =
        {
            { "all",          no_argument, 0, 'A'},
            { "progress",     no_argument, 0, 'B'},
            { "empty",        no_argument, 0, '0'},
            { "checksum",     no_argument, 0, 'C'},
            { "integrity",    no_argument, 0, 'I'},
            { "metadata",     no_argument, 0, 'M'},
            { "favicon",      no_argument, 0, 'F'},
            { "main",         no_argument, 0, 'P'},
            { "redundant",    no_argument, 0, 'R'},
            { "url_internal", no_argument, 0, 'U'},
            { "url_external", no_argument, 0, 'X'},
            { "mime",         no_argument, 0, 'E'},
            { "details",      no_argument, 0, 'D'},
            { "help",         no_argument, 0, 'H'},
            { "version",      no_argument, 0, 'V'},
            { 0, 0, 0, 0}
        };
        int option_index = 0;
        int c = getopt_long (argc, argv, "ACIMFPRUXEDHBVacimfpruxedhbv",
                             long_options, &option_index);
        //c = getopt (argc, argv, "ACMFPRUXED");
        if(c == -1)
            break;
        switch (c)
        {
        case 'A':
        case 'a':
            run_all = true;
            no_args = false;
            break;
        case '0':
            empty_check = true;
            break;
        case 'C':
        case 'c':
            checksum = true;
            no_args = false;
            break;
        case 'I':
        case 'i':
            integrity = true;
            no_args = false;
            break;
        case 'M':
        case 'm':
            metadata = true;
            no_args = false;
            break;
        case 'B':
        case 'b':
            progress.set_progress_report(true);
            break;
        case 'F':
        case 'f':
            favicon = true;
            no_args = false;
            break;
        case 'P':
        case 'p':
            main_page = true;
            no_args = false;
            break;
        case 'R':
        case 'r':
            redundant_data = true;
            no_args = false;
            break;
        case 'U':
        case 'u':
            url_check = true;
            no_args = false;
            break;
        case 'X':
        case 'x':
            url_check_external = true;
            no_args = false;
            break;
        case 'E':
        case 'e':
            mime_check = true;
            no_args = false;
            break;
        case 'D':
        case 'd':
            error_details = true;
            break;
        case 'H':
        case 'h':
            help=true;
            break;
        case '?':
            if (optopt == 'c')
            {
              std::cerr<<"Option "<<(char)optopt<<" requires an argument.\n";
              displayHelp();
            }
            else if ( isprint (optopt) )
              std::cerr<<"Unknown option `"<<( char )optopt<<"'.\n";
            else
            {
                std::cerr<<"Unknown option\n";
                displayHelp();
            }
            return 1;
        case 'V':
        case 'v':
          version();
          return 0;
        default:
            abort ();
        }
    }

    //Displaying Help for --help argument
    if(help)
    {
        displayHelp();
        return -1;
    }

    //If no arguments are given to the program, all the tests are performed.
    if ( run_all || no_args )
    {
        checksum = integrity = metadata = favicon = main_page = redundant_data =
          url_check = url_check_external = mime_check = empty_check = true;
    }

    //Obtaining filename from argument list
    filename = "";
    for(int i = 0; i < argc; i++)
    {
        if( (argv[i][0] != '-') && (i != 0))
        {
            filename = argv[i];
        }
    }
    if(filename == "")
    {
        std::cerr<<"No file provided as argument\n";
        displayHelp();
        return -1;
    }
    //Tests.
    try
    {
        std::cout << "[INFO] Checking zim file " << filename << std::endl;

        //Test 0: Low-level ZIM-file structure integrity checks
        if(integrity)
            test_integrity(filename, error);

        // Does it make sense to do the other checks if the integrity
        // check fails?
        zim::Archive archive( filename );

        //Test 1: Internal Checksum
        if(checksum) {
            if ( integrity ) {
                std::cout << "[INFO] Avoiding redundant checksum test"
                          << " (already performed by the integrity check)."
                          << std::endl;
            } else {
                test_checksum(archive, error);
            }
        }

        //Test 2: Metadata Entries:
        //The file is searched for the compulsory metadata entries.
        if(metadata)
            test_metadata(archive, error);

        //Test 3: Test for Favicon.
        if(favicon)
            test_favicon(archive, error);


        //Test 4: Main Page Entry
        if(main_page)
            test_mainpage(archive, error);

        /* Now we want to avoid to loop on the tests but on the article.
         *
         * If we loop of the tests we will have :
         *
         * for (test: tests) {
         *     for(article: articles) {
         *          data = article->getData();
         *          ...
         *     }
         * }
         *
         * And so we will get several the data of an article (and so decompression and so).
         * By looping on the articles first, we have :
         *
         * for (article: articles) {
         *     data = article->getData() {
         *     for (test: tests) {
         *         ...
         *     }
         * }
         */

        if ( redundant_data || url_check || url_check_external || empty_check )
          test_articles(archive, error, progress, redundant_data, url_check, url_check_external, empty_check);


        //Test 8: Verifying MIME Types
        //MIME Checks is intended to verify that all the MIME types of all different articles are listed in the file header.
        //As of now, there is no method in the existing zimlib to get the list of MIME types listed in the file header.
        //A bug has been reported for the above problem, and once the bug is fixed, it will be used to add MIME checks to the zimcheck tool.
        /*
                if(mime_check)
                {
                    std::cout<<"\nTest 8: Verifying MIME Types.. \n"<<std::flush;
                    progress.reset(articleCount);
                    test_=true;
                    for (zim::File::const_iterator it = f.begin(); it != f.end(); ++it)
                    {
                        progress.report();
                    }
                    if(test_)
                        std::cout<<"\nPass\n";
                    else
                    {
                        std::cout<<"\nFail\n";
                    }
                }
        */

        error.report(error_details);
        std::cout << "[INFO] Overall Test Status: ";
        if( error.overalStatus())
        {
            std::cout << "Pass" << std::endl;
            status_code = PASS;
        }
        else
        {
            std::cout << "Fail" << std::endl;
            status_code = FAIL;
        }
        time( &endTime );

        timeDiffference = difftime( endTime , startTime );
        std::cout << "[INFO] Total time taken by zimcheck: " << timeDiffference << " seconds."<<std::endl;

    }
    catch (const std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        status_code = EXCEPTION;
    }

    return status_code;
}
