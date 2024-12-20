/*
 * dEploid is used for deconvoluting Plasmodium falciparum genome from
 * mix-infected patient sample.
 *
 * Copyright (C) 2016-2017 University of Oxford
 *
 * Author: Sha (Joe) Zhu
 *
 * This file is part of dEploid.
 *
 * dEploid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <algorithm>     // std::min
#include <cassert>       // assert
#include <iostream>      // std::cout
#include "vcfReader.hpp"
#include "global.hpp"

// using namespace std;
using std::min;

/*! Initialize vcf file, search for the end of the vcf header.
 *  Extract the first block of data ( "buffer_length" lines ) into buff
 */
VcfReader::VcfReader(string fileName, string sampleName, bool extractPlaf) {
    /*! Initialize by read in the vcf header file */
    this->init(fileName);
    this->sampleName_ = sampleName;
    this->extractPlaf_ = extractPlaf;
    this->sampleColumnIndex_ = 0;
    this->readHeader();
    this->readVariants();
    this->getChromList();
    this->getIndexOfChromStarts();
    assert(this->doneGetIndexOfChromStarts_ == true);
    this->checkSortedPositions(fileName);
}


void VcfReader::checkFileCompressed() {
    FILE *f = NULL;
    f = fopen(this->fileName_.c_str(), "rb");
    if (f == NULL) {
        throw InvalidInputFile(this->fileName_);
    }

    unsigned char magic[2];

    size_t freadResults = fread(reinterpret_cast<void *>(magic), 1, 2, f);
    dout << "Check if vcf is compressed " << freadResults << std::endl;
    this->setIsCompressed((static_cast<int>(magic[0]) == 0x1f) &&
                              (static_cast<int>(magic[1]) == 0x8b));
    fclose(f);
}

void VcfReader::init(string fileName) {
    /*! Initialize other VcfReader class members
     */
    this->fileName_ = fileName;

    this->checkFileCompressed();

    if ( this->isCompressed() ) {
        this->inFileGz.open(this->fileName_.c_str(), std::ios::in);
    } else {
        this->inFile.open(this->fileName_.c_str(), std::ios::in);
    }
}


void VcfReader::finalize() {
    for (size_t i = 0; i < this->variants.size(); i++) {
        this->refCount.push_back(static_cast<double>(this->variants[i].ref));
        this->altCount.push_back(static_cast<double>(this->variants[i].alt));
        this->vqslod.push_back(this->variants[i].vqslod);
        this->plaf.push_back(this->variants[i].plaf);
    }

    if ( this->isCompressed() ) {
        this->inFileGz.close();
    } else {
        this->inFile.close();
    }
}


void VcfReader::readHeader() {
    if (this->isCompressed()) {
        if (!inFileGz.good()) {
            throw InvalidInputFile(this->fileName_);
        }
    } else {
        if (!inFile.good()) {
            throw InvalidInputFile(this->fileName_);
        }
    }

    if (this->isCompressed()) {
        getline(inFileGz, this->tmpLine_);
    } else {
        getline(inFile, this->tmpLine_);
    }

    while (this->tmpLine_.size() > 0) {
        if (this->tmpLine_[0] == '#') {
            if (this->tmpLine_[1] == '#') {
                this->headerLines.push_back(this->tmpLine_);
                if (this->isCompressed()) {
                    getline(inFileGz, this->tmpLine_);
                } else {
                    getline(inFile, this->tmpLine_);
                }
            } else {
                this->checkFeilds();
                break;  // end of the header
            }
        } else {
            this->checkFeilds();
        }
    }

    dout << " There are " << this->headerLines.size()
         << " lines in the header." <<std::endl;
}


void VcfReader::checkFeilds() {
    size_t feild_start = 0;
    size_t field_end = 0;
    size_t field_index = 0;

    while (field_end < this->tmpLine_.size()) {
        field_end = min(this->tmpLine_.find('\t', feild_start),
                        this->tmpLine_.find('\n', feild_start));
        this->tmpStr_ = this->tmpLine_.substr(feild_start,
                                              field_end-feild_start);
        string correctFieldValue;
        switch ( field_index ) {
            case 0: correctFieldValue = "#CHROM";  break;
            case 1: correctFieldValue =  "POS";    break;
            case 2: correctFieldValue =  "ID";     break;
            case 3: correctFieldValue =  "REF";    break;
            case 4: correctFieldValue =  "ALT";    break;
            case 5: correctFieldValue =  "QUAL";   break;
            case 6: correctFieldValue =  "FILTER"; break;
            case 7: correctFieldValue =  "INFO";   break;
            case 8: correctFieldValue =  "FORMAT"; break;
        }

        if (this->tmpStr_ != correctFieldValue && field_index < 9) {
            throw VcfInvalidHeaderFieldNames(correctFieldValue, this->tmpStr_);
        }

        if ((field_index == 9) & (this->sampleName_ == "")) {
            this->sampleName_ = this->tmpStr_;
        }

        if (this->tmpStr_ == this->sampleName_) {
            sampleColumnIndex_ = field_index;
            break;
        }

        feild_start = field_end+1;
        field_index++;
    }  // End of while loop: field_end < line.size()

    if (sampleColumnIndex_ == 0) {
        throw InvalidSampleInVcf(this->sampleName_, this->fileName_);
    }
    assert(sampleColumnIndex_ != 0);
    assert(printSampleName());
}


void VcfReader::readVariants() {
    if (this->isCompressed()) {
        getline(inFileGz, this->tmpLine_);
    } else {
        getline(inFile, this->tmpLine_);
    }
    while (inFile.good() && this->tmpLine_.size() > 0) {
        VariantLine newVariant(this->tmpLine_, this->sampleColumnIndex_,
            this->extractPlaf_);
        // check variantLine quality
        this->variants.push_back(newVariant);
        if (this->isCompressed()) {
            getline(inFileGz, this->tmpLine_);
        } else {
            getline(inFile, this->tmpLine_);
        }
    }
}


void VcfReader::getChromList() {
    this->chrom_.clear();
    this->position_.clear();

    assert(this->chrom_.size() == (size_t)0);
    assert(this->position_.size() == (size_t)0);

    string previousChrom("");
    vector <int> positionOfChrom_;

    for (size_t i = 0; i < this->variants.size() ; i++) {
        if (previousChrom != this->variants[i].chromStr &&
            previousChrom.size() > (size_t)0) {
            this->chrom_.push_back(previousChrom);
            this->position_.push_back(positionOfChrom_);
            positionOfChrom_.clear();
        }
        positionOfChrom_.push_back(
            stoi(this->variants[i].posStr.c_str(), NULL));
        previousChrom = this->variants[i].chromStr;
    }

    this->chrom_.push_back(previousChrom);
    this->position_.push_back(positionOfChrom_);
    assert(this->position_.size() == this->chrom_.size());
}


void VcfReader::removeMarkers() {
    assert(this->keptVariants.size() == (size_t)0);
    for (auto const &value : this->indexOfContentToBeKept) {
        this->keptVariants.push_back(this->variants[value]);
    }
    this->variants.clear();
    this->variants = this->keptVariants;
    this->keptVariants.clear();
    this->nLoci_ = this->variants.size();
    dout << " Vcf number of loci kept = " << this->nLoci_ << std::endl;
}


void VcfReader::findLegitSnpsGivenVQSLOD(double vqslodThreshold) {
    this->legitVqslodAt.clear();
    assert(legitVqslodAt.size() == 0);
    for (size_t i = 0; i < this->vqslod.size(); i++) {
        if (this->vqslod[i] > vqslodThreshold) {
            this->legitVqslodAt.push_back(i);
        }
    }
}


void VcfReader::findLegitSnpsGivenVQSLODHalf(double vqslodThreshold) {
    this->legitVqslodAt.clear();
    assert(legitVqslodAt.size() == 0);

    for (size_t chromI = 0;
         chromI < this->indexOfChromStarts_.size(); chromI++) {
        size_t start = this->indexOfChromStarts_[chromI];
        size_t length = this->position_[chromI].size();
        // if (chromI%2 == 0) {
        if (chromI > 10) {
            for ( size_t ii = start ; ii < (start+length); ii++ ) {
                if (this->vqslod[ii] > vqslodThreshold) {
                    // std::cout << ii <<std::std::endl;
                    this->legitVqslodAt.push_back(ii);
                }
            }
        }
    }
}


VariantLine::VariantLine(string tmpLine, size_t sampleColumnIndex,
    bool extractPlaf) {
    this->init(tmpLine, sampleColumnIndex, extractPlaf);

    while (fieldEnd_ < this->tmpLine_.size()) {
        fieldEnd_ = min(this->tmpLine_.find('\t', feildStart_),
                        this->tmpLine_.find('\n', feildStart_));
        this->tmpStr_ = this->tmpLine_.substr(feildStart_,
                                              fieldEnd_-feildStart_);
        switch (fieldIndex_) {
            case 0: this->extract_field_CHROM();   break;
            case 1: this->extract_field_POS();    break;
            case 2: this->extract_field_ID();     break;
            case 3: this->extract_field_REF() ;   break;
            case 4: this->extract_field_ALT() ;   break;
            case 5: this->extract_field_QUAL() ;   break;
            case 6: this->extract_field_FILTER();  break;
            case 7: this->extract_field_INFO();    break;
            case 8: this->extract_field_FORMAT();  break;
        }

        if (fieldIndex_ == this->sampleColumnIndex_) {
            this->extract_field_VARIANT();
            break;
        }
        feildStart_ = fieldEnd_+1;
        fieldIndex_++;
    }
}


void VariantLine::init(string tmpLine, size_t sampleColumnIndex,
    bool extractPlaf) {
    this->tmpLine_ = tmpLine;
    this->feildStart_ = 0;
    this->fieldEnd_ = 0;
    this->fieldIndex_  = 0;
    this->adFieldIndex_ = -1;
    this->sampleColumnIndex_ = sampleColumnIndex;
    this->extractPlaf_ = extractPlaf;
}


void VariantLine::extract_field_CHROM() {
    this->chromStr = this->tmpStr_;
}


void VariantLine::extract_field_POS() {
    this->posStr = this->tmpStr_;
}


void VariantLine::extract_field_ID() {
    this->idStr = this->tmpStr_;
}


void VariantLine::extract_field_REF() {
    this->refStr = this->tmpStr_;
}


void VariantLine::extract_field_ALT() {
    this->altStr = this->tmpStr_;
}


void VariantLine::extract_field_QUAL() {  // Check for PASS
    this->qualStr = this->tmpStr_;
}


void VariantLine::extract_field_FILTER() {
    this->filterStr = this->tmpStr_;
}

void VariantLine::extract_field_INFO() {
    this->infoStr = this->tmpStr_;
    bool vqslodNotFound = true;
    size_t feild_start = 0;
    size_t field_end = 0;
    int field_index = 0;

    while (field_end < this->tmpStr_.size()) {
        field_end = min(this->tmpStr_.find(';', feild_start),
                        this->tmpStr_.find('\t', feild_start));
        string filterFiledStr = this->tmpStr_.substr(feild_start,
                                                     field_end-feild_start);
        size_t eqIndex = filterFiledStr.find('=', 0);
        string filterFiledName = filterFiledStr.substr(0, eqIndex);
        if ( "VQSLOD" == filterFiledName ) {
            vqslodNotFound = false;
            vqslod = stod(filterFiledStr.substr(eqIndex+1,
                                                filterFiledStr.size()));
        }

        if ( ("AF" == filterFiledName) & (this->extractPlaf_) ) {
            plaf = stod(filterFiledStr.substr(eqIndex+1,
                                                filterFiledStr.size()));
        }

        feild_start = field_end+1;
        field_index++;
    }

    if (vqslodNotFound) {
        throw VcfVQSLODNotFound(this->tmpStr_);
    }

    assert(vqslodNotFound == false);
}


void VariantLine::extract_field_FORMAT() {
    this->formatStr = this->tmpStr_;
    size_t feild_start = 0;
    size_t field_end = 0;
    size_t field_index = 0;

    while (field_end < this->formatStr.size()) {
        field_end = min(this->formatStr.find(':', feild_start),
                        this->formatStr.find('\n', feild_start));
        if ( "AD" == this->formatStr.substr(feild_start,
                                            field_end-feild_start)) {
            adFieldIndex_ = field_index;
            break;
        }
        feild_start = field_end+1;
        field_index++;
    }
    if (adFieldIndex_ == -1) {
        throw VcfCoverageFieldNotFound(this->tmpStr_);
    }
    assert(adFieldIndex_ > -1);
}


int maybe_dot_to_integer(const string& s) {
    if (s == ".")
        return 0;
    else
        return stoi(s);
}

int n_fields(const string& s, char delim = ',') {
    int count = 0;
    for (auto ch : s)
        if (ch == delim)
            count++;
    return count+1;
}

void VariantLine::extract_field_VARIANT() {
    size_t feild_start = 0;
    size_t field_end = 0;
    int field_index = 0;

    while (field_end < this->tmpStr_.size()) {
        field_end = min(this->tmpStr_.find(':', feild_start),
                        this->tmpStr_.find('\n', feild_start));
        if (field_index == adFieldIndex_) {
            string adStr = this->tmpStr_.substr(feild_start,
                                                field_end-feild_start);
            try {
                int n = n_fields(adStr);
                if (n != 2)
                    throw std::runtime_error(
                        "there should be exactly 2 AD entries, but found " +
                        std::to_string(n) +
                        ".\n   Wrong number of ALT alleles!.");

                size_t commaIndex = adStr.find(',', 0);
                auto ref_AD = adStr.substr(0, commaIndex);
                auto alt_AD = adStr.substr(commaIndex+1, adStr.size());

                ref = maybe_dot_to_integer(ref_AD);
                alt = maybe_dot_to_integer(alt_AD);
                break;
            }
            catch (const std::exception& e) {
              throw std::runtime_error(
                  "Error parsing vcf AD field: '" +
                    adStr + "':  " + e.what() + "\n");
            }
        }
        feild_start = field_end+1;
        field_index++;
    }
}

/*
void VcfReader::findLegitSnpsGivenVQSLODandWsfGt0(double vqslodThreshold) {
    assert(legitVqslodAt.size() == 0);
    for (size_t i = 0; i < this->vqslod.size(); i++) {
        if (this->vqslod[i] > vqslodThreshold & this->altCount[i] > 0) {
            this->legitVqslodAt.push_back(i);
        }
    }
}
*/
