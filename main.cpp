/*
 * dEploid is used for deconvoluting Plasmodium falciparum genome from
 * mix-infected patient sample. DEploid-vcf-lib is a submodule for
 * reading the vcf files and reference panel.
 *
 * Copyright (C) 2018 University of Oxford
 *
 * Author: Sha (Joe) Zhu
 *
 * This file is part of DEploid-vcf-lib.
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

#include <iostream>  // std::cout
#include "vcfReader.hpp"

int main(int argc, char *argv[]) {
    try {
        VcfReader vcfReader(argv[1], "PG0390-C");
//        vcfReader.printSampleName();
        return EXIT_SUCCESS;
    }
    catch (const exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
