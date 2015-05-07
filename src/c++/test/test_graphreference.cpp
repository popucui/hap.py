// -*- mode: c++; indent-tabs-mode: nil; -*-
//
// Copyright (c) 2010-2015 Illumina, Inc.
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * \brief Test cases for the graph reference structure
 *
 * \file test_graphreference.cpp
 * \author Peter Krusche
 * \email pkrusche@illumina.com
 *
 */

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/filesystem/path.hpp>

#include "GraphReference.hh"
#include "helpers/GraphUtil.hh"

#include <iostream>
#include <vector>
#include <set>

using namespace variant;
using namespace haplotypes;


BOOST_AUTO_TEST_CASE(graphReferenceBasic)
{
    boost::filesystem::path p(__FILE__);
    boost::filesystem::path tp = p.parent_path()
                                   .parent_path()   // test
                                   .parent_path()   // c++
                                    / boost::filesystem::path("data");

    std::string datapath = tp.string();

    GraphReference gr((datapath + "/refgraph1.vcf.gz").c_str(), 
                      "NA12877", (datapath + "/chrQ.fa").c_str());
    
    std::vector<Haplotype> target;
    gr.enumeratePaths(target, "chrQ", 0, 24);

    BOOST_CHECK_EQUAL(target.size(), (size_t)8);

    std::set<std::string> expected;

    // A[AAC/]CC[GGG]AAA[C/G] [C|T][C|T][T|G]AACCCGG[C|T]TTTG

    expected.insert("ACCGGGAAACCCTAACCCGGCTTTG");
    expected.insert("ACCGGGAAACTTGAACCCGGTTTTG");
    expected.insert("ACCGGGAAAGCCTAACCCGGCTTTG");
    expected.insert("ACCGGGAAAGTTGAACCCGGTTTTG");
    expected.insert("AAACCCGGGAAACCCTAACCCGGCTTTG");
    expected.insert("AAACCCGGGAAACTTGAACCCGGTTTTG");
    expected.insert("AAACCCGGGAAAGCCTAACCCGGCTTTG");
    expected.insert("AAACCCGGGAAAGTTGAACCCGGTTTTG");

    bool err = false;
    for(Haplotype const & h : target)
    {
        BOOST_CHECK_EQUAL(expected.count(h.seq(0, 24)), (size_t)1);
        if (expected.count(h.seq(0, 24)) == 0)
        {
            std::cerr << "[E] Missing a haplotype: " << h.seq(0, 24) << "\n";
            err = true;
        }
    }
    if (err)
    {
        for (Haplotype const & h : target)
        {
            std::cerr << "[E] hap list: " << h.seq(0, 24) << "\n";
        }
    }
}

BOOST_AUTO_TEST_CASE(graphReferencePhased)
{
    boost::filesystem::path p(__FILE__);
    boost::filesystem::path tp = p.parent_path()
                                   .parent_path()   // test
                                   .parent_path()   // c++
                                    / boost::filesystem::path("data");

    std::string datapath = tp.string();

    GraphReference gr((datapath + "/refgraph1.vcf.gz").c_str(), 
                      "NA12877", (datapath + "/chrQ.fa").c_str());
    
    std::vector<Haplotype> target;

    // in this region, we only have phased variants.
    // we should only get two paths
    gr.enumeratePaths(target, "chrQ", 10, 24);

    BOOST_CHECK_EQUAL(target.size(), (size_t)2);

    std::set<std::string> expected;
    expected.insert("CCTAACCCGGCTTTG");
    expected.insert("TTGAACCCGGTTTTG");

    for(Haplotype const & h : target)
    {
        BOOST_CHECK_EQUAL(expected.count(h.seq(10, 24)), (size_t)1);
    }
}

BOOST_AUTO_TEST_CASE(graphReferenceWithSink)
{
    boost::filesystem::path p(__FILE__);
    boost::filesystem::path tp = p.parent_path()
                                   .parent_path()   // test
                                   .parent_path()   // c++
                                    / boost::filesystem::path("data");

    std::string datapath = tp.string();

    GraphReference gr((datapath + "/refgraph1.vcf.gz").c_str(), 
                      "NA12877", (datapath + "/chrQ.fa").c_str());
    
    std::vector<ReferenceNode> nodes;
    std::vector<ReferenceEdge> edges;
    gr.makeGraph("chrQ", 0, 24, nodes, edges);
    
    std::vector<Haplotype> target;

    gr.enumeratePaths("chrQ", 0, 24, nodes, edges, target, 0, -1, 6);

    BOOST_CHECK_EQUAL(target.size(), (size_t)4);

    std::set<std::string> expected;
    expected.insert("ACCGGGAAACCCAAACCCGGGTTTG");
    expected.insert("ACCGGGAAAGCCAAACCCGGGTTTG");
    expected.insert("AAACCCGGGAAACCCAAACCCGGGTTTG");
    expected.insert("AAACCCGGGAAAGCCAAACCCGGGTTTG");
    bool err = false;
    for(Haplotype const & h : target)
    {
        BOOST_CHECK_EQUAL(expected.count(h.seq(0, 24)), (size_t)1);
        if (expected.count(h.seq(0, 24)) == 0)
        {
            std::cerr << "[E] Missing a haplotype: " << h.seq(0, 24) << "\n";
            err = true;
        }
    }
    if (err)
    {
        for (Haplotype const & h : target)
        {
            std::cerr << "[E] hap list: " << h.seq(0, 24) << "\n";
        }
    }

}

BOOST_AUTO_TEST_CASE(graphNodesUsed)
{
    boost::filesystem::path p(__FILE__);
    boost::filesystem::path tp = p.parent_path()
                                   .parent_path()   // test
                                   .parent_path()   // c++
                                    / boost::filesystem::path("data");

    std::string datapath = tp.string();

    GraphReference gr((datapath + "/refgraph1.vcf.gz").c_str(), 
                      "NA12877", (datapath + "/chrQ.fa").c_str());
    
    std::vector<ReferenceNode> nodes;
    std::vector<ReferenceEdge> edges;
    size_t nhets = 0;

    gr.makeGraph("chrQ", 0, 24, nodes, edges, &nhets);

    BOOST_CHECK_EQUAL(nhets, (size_t)2);
    
    std::vector<Haplotype> target;
    std::vector<std::string> nodes_used;

    gr.enumeratePaths("chrQ", 0, 24, nodes, edges, target, 0, -1, -1, &nodes_used);

    std::set<std::string> expected;
    
    expected.insert("chrQ:0-20:ACCGGGAAACCCTAACCCGGC:101010101011101");
    expected.insert("chrQ:0-20:ACCGGGAAACTTGAACCCGGT:110101010011101");
    expected.insert("chrQ:0-20:ACCGGGAAAGCCTAACCCGGC:101010101101101");
    expected.insert("chrQ:0-20:ACCGGGAAAGTTGAACCCGGT:110101010101101");
    expected.insert("chrQ:5-20:CGGGAAACCCTAACCCGGC:101010101011011");
    expected.insert("chrQ:5-20:CGGGAAACTTGAACCCGGT:110101010011011");
    expected.insert("chrQ:5-20:CGGGAAAGCCTAACCCGGC:101010101101011");
    expected.insert("chrQ:5-20:CGGGAAAGTTGAACCCGGT:110101010101011");

    size_t is = 0;
    for(std::string const & s : nodes_used)
    {
        // std::cerr << target[is].repr() + ":" + s << "\n";
        BOOST_CHECK_EQUAL(expected.count(target[is].repr() + ":" + s), (size_t)1);
        ++is;
    }
    BOOST_CHECK_EQUAL(is, expected.size());
}

BOOST_AUTO_TEST_CASE(graphNodesUsedPhased)
{
    boost::filesystem::path p(__FILE__);
    boost::filesystem::path tp = p.parent_path()
                                   .parent_path()   // test
                                   .parent_path()   // c++
                                    / boost::filesystem::path("data");

    std::string datapath = tp.string();

    GraphReference gr((datapath + "/refgraph1.vcf.gz").c_str(), 
                      "NA12877", (datapath + "/chrQ.fa").c_str());
    
    std::vector<ReferenceNode> nodes;
    std::vector<ReferenceEdge> edges;
    gr.makeGraph("chrQ", 11, 24, nodes, edges);
    
    std::vector<Haplotype> target;
    std::vector<std::string> nodes_used;

    gr.enumeratePaths("chrQ", 11, 25, nodes, edges, target, 0, -1, -1, &nodes_used);

    std::set<std::string> expected;

    expected.insert("chrQ:11-20:TGAACCCGGT:11010101");
    expected.insert("chrQ:12-20:TAACCCGGC:10101011");

    size_t is = 0;
    for(std::string const & s : nodes_used)
    {
        // std::cerr << target[is].repr() + ":" + s << "\n";
        BOOST_CHECK_EQUAL(expected.count(target[is].repr() + ":" + s), (size_t)1);
        ++is;
    }
    BOOST_CHECK_EQUAL(is, expected.size());
}

BOOST_AUTO_TEST_CASE(graphHomref)
{
    boost::filesystem::path p(__FILE__);
    boost::filesystem::path tp = p.parent_path()
                                   .parent_path()   // test
                                   .parent_path()   // c++
                                    / boost::filesystem::path("data");

    std::string datapath = tp.string();

    GraphReference gr((datapath + "/refgraph1.vcf.gz").c_str(), 
                      "NA12877", (datapath + "/chrQ.fa").c_str());
    
    std::vector<ReferenceNode> nodes;
    std::vector<ReferenceEdge> edges;
    // this region is homref
    gr.makeGraph("chrQ", 27, 36, nodes, edges);
    
    std::vector<Haplotype> target;
    std::vector<std::string> nodes_used;

    gr.enumeratePaths("chrQ", 11, 25, nodes, edges, target, 0, -1, -1, &nodes_used);

    std::set<std::string> expected;
    expected.insert("chrQ:novar:01");

    size_t is = 0;
    for(std::string const & s : nodes_used)
    {
        // std::cerr << (target[is].repr() + ":" + s) << std::endl;
        BOOST_CHECK_EQUAL(expected.count(target[is].repr() + ":" + s), (size_t)1);
        ++is;
    }
    BOOST_CHECK_EQUAL(is, expected.size());
}