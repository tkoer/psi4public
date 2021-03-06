/*
 *@BEGIN LICENSE
 *
 * PSI4: an ab initio quantum chemistry software package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *@END LICENSE
 */

/*!
    \defgroup MINTS libmints: Integral library
    \ingroup MINTS
*/
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>

#include <libciomr/libciomr.h>
#include <libparallel/parallel.h>
#include <psifiles.h>

#include "vector3.h"
#include "molecule.h"
#include "basisset.h"
#include "dimension.h"
#include "sobasis.h"
#include "integral.h"
#include "symmetry.h"
#include "gshell.h"
#include "factory.h"
#include "basisset_parser.h"
#include "pointgrp.h"
#include "wavefunction.h"
#include "coordentry.h"

using namespace std;
using namespace psi;
using namespace boost;

bool BasisSet::initialized_shared_ = false;

std::vector<Vector3> BasisSet::exp_ao[LIBINT_MAX_AM];

namespace {
bool has_ending (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}
}

// Constructs a zero AO basis set
BasisSet::BasisSet()
{
    if (initialized_shared_ == false)
        initialize_singletons();
    initialized_shared_ = true;

    // Add a dummy atom at the origin, to hold this basis function
    molecule_ = boost::shared_ptr<Molecule>(new Molecule);
    molecule_->add_atom(0, 0.0, 0.0, 0.0);
    // Fill with data representing a single S function, at the origin, with 0 exponent
    n_uprimitive_ = 1;
    n_shells_ = 1;
    nprimitive_ = 1;
    nao_ = 1;
    nbf_ = 1;
    n_prim_per_shell_ = new int[1];
    uexponents_ = new double[1];
    ucoefficients_ = new double[1];
    uerd_coefficients_ = new double[1];
    uoriginal_coefficients_ = new double[1];
    shell_first_ao_ = new int[1];
    shell_first_basis_function_ = new int[1];
    shells_ = new GaussianShell[1];
    ao_to_shell_ = new int[1];
    function_to_shell_ = new int[1];
    function_center_ = new int[1];
    shell_center_ = new int[1];
    center_to_nshell_ = new int[1];
    center_to_shell_ = new int[1];
    xyz_ = new double[3];
    n_prim_per_shell_[0] = 1;
    uexponents_[0] = 0.0;
    ucoefficients_[0] = 1.0;
    uoriginal_coefficients_[0] = 1.0;
    shell_first_ao_[0] = 0;
    shell_first_basis_function_[0] = 0;
    ao_to_shell_[0] = 0;
    function_to_shell_[0] = 0;
    function_center_[0] = 0;
    shell_center_[0] = 0;
    center_to_nshell_[0] = 1;
    center_to_shell_[0] = 0;
    puream_ = 0;
    max_am_ = 0;
    max_nprimitive_ = 1;
    xyz_[0] = 0.0;
    xyz_[1] = 0.0;
    xyz_[2] = 0.0;
    shells_[0] = GaussianShell(0, nprimitive_, uoriginal_coefficients_, ucoefficients_, uerd_coefficients_,
                               uexponents_, GaussianType(0), 0, xyz_, 0);
}

boost::shared_ptr<BasisSet> BasisSet::build(boost::shared_ptr<Molecule> molecule,
                                            const std::vector<ShellInfo>& shells)
{
    //TODO fixme!!!
    boost::shared_ptr<BasisSet> basis(new BasisSet());
//    basis->molecule_ = molecule;
//    basis->shells_ = shells;
//    basis->refresh();

    return basis;
}

void BasisSet::initialize_singletons()
{
    // Populate the exp_ao arrays
    for (int l=0; l<LIBINT_MAX_AM; ++l) {
        for (int i=0; i<=l; ++i) {
            int x = l-i;
            for (int j=0; j<=i; ++j) {
                int y = i-j;
                int z = j;

                Vector3 xyz_ao(x, y, z);
                BasisSet::exp_ao[l].push_back(xyz_ao);
            }
        }
    }
}

boost::shared_ptr<Molecule> BasisSet::molecule() const
{
    return molecule_;
}

void BasisSet::print(FILE *out) const
{
    if (WorldComm->me() == 0) {
        fprintf(out, "  Basis Set: %s\n", name_.c_str());
        fprintf(out, "    Number of shells: %d\n", nshell());
        fprintf(out, "    Number of basis function: %d\n", nbf());
        fprintf(out, "    Number of Cartesian functions: %d\n", nao());
        fprintf(out, "    Spherical Harmonics?: %s\n", has_puream() ? "true" : "false");
        fprintf(out, "    Max angular momentum: %d\n\n", max_am());
    }
}

void BasisSet::print_by_level(FILE* out, int level) const
{
    if (level < 1)
        return;
    else if (level == 1)
        print(out);
    else if (level == 2)
        print_summary(out);
    else if (level > 2)
        print_detail(out);
}

void BasisSet::print_summary(FILE* out) const
{
    if (WorldComm->me() == 0) {
        fprintf(out, "  -AO BASIS SET INFORMATION:\n");
        fprintf(out, "    Name                   = %s\n", name_.c_str());
        fprintf(out, "    Total number of shells = %d\n", nshell());
        fprintf(out, "    Number of primitives   = %d\n", nprimitive_);
        fprintf(out, "    Number of AO           = %d\n", nao_);
        fprintf(out, "    Number of SO           = %d\n", nbf_);
        fprintf(out, "    Maximum AM             = %d\n", max_am_);
        fprintf(out, "    Spherical Harmonics    = %s\n", (puream_ ? "TRUE" : "FALSE"));
        fprintf(out, "\n");

        fprintf(out,"  -Contraction Scheme:\n");
        fprintf(out, "    Atom   Type   All Primitives // Shells:\n");
        fprintf(out, "   ------ ------ --------------------------\n");
    }

    int *nprims = new int[max_am_ + 1];
    int *nunique = new int[max_am_ + 1];
    int *nshells = new int[max_am_ + 1];
    char *amtypes = new char[max_am_ + 1];

    for (int A = 0; A < molecule_->natom(); A++) {

        memset((void*) nprims , '\0', (max_am_ + 1) * sizeof(int));
        memset((void*) nunique, '\0', (max_am_ + 1) * sizeof(int));
        memset((void*) nshells, '\0', (max_am_ + 1) * sizeof(int));

        if (WorldComm->me() == 0) {
            fprintf(out, "    %4d    ", A+1);
            fprintf(out, "%2s     ", molecule_->symbol(A).c_str());
        }

        int first_shell = center_to_shell_[A];
        int n_shell = center_to_nshell_[A];

        for (int Q = 0; Q < n_shell; Q++) {
            const GaussianShell& shell = shells_[Q + first_shell];
            nshells[shell.am()]++;
            nunique[shell.am()]+= shell.nprimitive();
            nprims [shell.am()]+= shell.nprimitive();
            amtypes[shell.am()] = shell.amchar();
        }

        // All Primitives
        for (int l = 0; l < max_am_ + 1; l++) {
            if (nprims[l] == 0)
                continue;
            if (WorldComm->me() == 0)
                fprintf(out, "%d%c ", nprims[l], amtypes[l]);
        }
        // Shells
        if (WorldComm->me() == 0)
            fprintf(out, "// ");
        for (int l = 0; l < max_am_ + 1; l++) {
            if (nshells[l] == 0)
                continue;
            if (WorldComm->me() == 0)
                fprintf(out, "%d%c ", nshells[l], amtypes[l]);
        }
        if (WorldComm->me() == 0)
            fprintf(out, "\n");
    }
    if (WorldComm->me() == 0)
        fprintf(out, "\n");

    delete[] nprims;
    delete[] nunique;
    delete[] nshells;
    delete[] amtypes;
}

void BasisSet::print_detail(FILE* out) const
{
    print_summary(out);

    if (WorldComm->me() == 0) {
        fprintf(out, "  ==> AO Basis Functions <==\n");
        fprintf(out, "\n");
        fprintf(out, "    [ %s ]\n",name_.c_str());
        if (has_puream())
            fprintf(out, "    spherical\n");
        else
            fprintf(out, "    cartesian\n");
        fprintf(out, "    ****\n");
    }

    for (int uA = 0; uA < molecule_->nunique(); uA++) {
        const int A = molecule_->unique(uA);
        if (WorldComm->me() == 0) {
            fprintf(out, "   %2s %3d\n",molecule_->symbol(A).c_str(),A+1);
        }

        int first_shell = center_to_shell_[A];
        int n_shell = center_to_nshell_[A];

        for (int Q = 0; Q < n_shell; Q++)
            shells_[Q + first_shell].print(out);

        if (WorldComm->me() == 0){
            fprintf(out, "    ****\n");
        }
    }
    if (WorldComm->me() == 0){
        fprintf(out, "\n");
    }
}

const GaussianShell& BasisSet::shell(int si) const
{
    if (si < 0 || si > nshell())
        throw PSIEXCEPTION("BasisSet::shell: requested shell is out-of-bounds.");
    return shells_[si];
}

const GaussianShell& BasisSet::shell(int center, int si) const
{
    return shell(center_to_shell_[center] + si);
}

boost::shared_ptr<BasisSet> BasisSet::zero_ao_basis_set()
{
    // In the new implementation, we simply call the default constructor
    boost::shared_ptr<BasisSet> new_basis(new BasisSet());
    return new_basis;
}

//boost::shared_ptr<SOBasisSet> BasisSet::zero_so_basis_set(const boost::shared_ptr<IntegralFactory>& factory)
//{
//    boost::shared_ptr<BasisSet> zero = BasisSet::zero_ao_basis_set();
//    boost::shared_ptr<SOBasisSet> sozero(new SOBasisSet(zero, factory));
//    return sozero;
//}

boost::shared_ptr<BasisSet> BasisSet::construct(const boost::shared_ptr<BasisSetParser>& parser,
        const boost::shared_ptr<Molecule>& mol,
        const std::string& type)
{
    // Update geometry in molecule, if there is a problem an exception is thrown.
    mol->update_geometry();

    // For each one try to load the basis set
    const list<string>& user_list = Process::environment.user_basis_files;

    // Map of GaussianShells
    //  basis           atom        gaussian shells
    typedef map<string, map<string, vector<ShellInfo> > > map_ssv;
    typedef map<string, vector<ShellInfo> > map_sv;
    map_ssv basis_atom_shell;

    map<string, int> names;

    for (int atom=0; atom<mol->natom(); ++atom) {

        const string& symbol = mol->atom_entry(atom)->symbol();
        const string& basisname = mol->atom_entry(atom)->basisset(type);

        if (basisname.empty())
            throw PSIEXCEPTION("BasisSet::construct: No basis set specified for "
                               +symbol+ " and " +type+" type.");

        names[basisname] = 1;

        // Add basisname, symbol to the list by clearing the vector.
        basis_atom_shell[basisname][symbol].clear();
    }

    BOOST_FOREACH(map_ssv::value_type& basis, basis_atom_shell)
    {
         bool not_found = true;

        BOOST_FOREACH(string user_file, user_list)
        {
            boost::filesystem::path bf_path;
            bf_path = boost::filesystem::system_complete(user_file);
            // Load in the basis set and remove it from atomsymbol_to_basisname
            vector<string> file = parser->load_file(bf_path.string(), basis.first);

            BOOST_FOREACH(map_sv::value_type& atom, basis.second) {
                string symbol = atom.first;

                // Don't even look, if this has already been found
                if(!basis_atom_shell[basis.first][symbol].empty()) continue;

                try {
                    // Need to wrap this is a try catch block
                    basis_atom_shell[basis.first][symbol] = parser->parse(symbol, file);

                    if (WorldComm->me() == 0)
                        fprintf(outfile, "  Basis set %s for %s read from %s\n",
                                basis.first.c_str(), symbol.c_str(), user_file.c_str());
                    not_found = false;
                }
                catch (BasisSetNotFound& e) {
                    // This is thrown when load_file fails
                    if (WorldComm->me() == 0)
                        fprintf(outfile, "  Unable to find %s for %s in %s.\n",
                                basis.first.c_str(), symbol.c_str(), user_file.c_str());
                    not_found = true;
                }
            }
        }

        string filename = make_filename(basis.first);
        string path = Process::environment("PSIDATADIR");
        vector<string> file;

        try {
            // Don't even look, if this has already been found
            BOOST_FOREACH(map_sv::value_type& atom, basis.second) {
                string symbol = atom.first;
                // Don't bother looking if we've already found this
                if (atom.second.empty()){
                    if(file.empty()) file = parser->load_file(path + "/basis/" + filename);
                    // If not found this will throw...let it.
                    basis_atom_shell[basis.first][symbol] = parser->parse(symbol, file);
                }
            }
        }
        catch (BasisSetFileNotFound& e) {
            fprintf(outfile, " Unable to load %s from the default Psi4 basis set library.\n", filename.c_str());
            throw PSIEXCEPTION("  Unable to load "+ filename + " from the default Psi4 basis set library.");
        }
    }

    boost::shared_ptr<BasisSet> basisset(new BasisSet(type, mol, basis_atom_shell));

    //TODO ACS is this still needed?
    // This step is very important. Without it the basis set is useless.
    basisset->refresh();

    basisset->name_.clear();
    for (map<string, int>::iterator iter = names.begin(), end = names.end();
         iter != end;
         ++iter) {
        basisset->name_ += (*iter).first + " + ";
    }
    if (has_ending(basisset->name_, " + "))
        basisset->name_ = basisset->name_.substr(0, basisset->name_.length()-3);

    return basisset;
}

BasisSet::BasisSet(const std::string& basistype, SharedMolecule mol,
                   std::map<std::string, std::map<std::string, std::vector<ShellInfo> > > &shell_map):
    molecule_(mol),
    name_(basistype)
{
    // Singletons
    if (initialized_shared_ == false)
        initialize_singletons();
    initialized_shared_ = true;

    int natom = molecule_->natom();

    /// These will tell us where the primitives for [basis][symbol] start and end, in the compact array
    std::map<std::string, std::map<std::string, int > >  primitive_start;
    std::map<std::string, std::map<std::string, int > >  primitive_end;

    /*
     * First, loop over the unique primitives, and store them
     */
    std::vector<double> uexps;
    std::vector<double> ucoefs;
    std::vector<double> uoriginal_coefs;
    std::vector<double> uerd_coefs;
    n_uprimitive_ = 0;
    std::map<std::string, std::map<std::string, std::vector<ShellInfo> > >::iterator  basis_iter;
    for(basis_iter = shell_map.begin(); basis_iter != shell_map.end(); ++basis_iter){
        const std::string &basis = basis_iter->first;
        std::map<std::string, std::vector<ShellInfo> > &symbol_map = shell_map[basis];
        std::map<std::string, std::vector<ShellInfo> >::iterator symbol_iter;
        for(symbol_iter = symbol_map.begin(); symbol_iter != symbol_map.end(); ++symbol_iter){
            const std::string &symbol = symbol_iter->first;
            vector<ShellInfo>& shells = symbol_map[symbol];
            primitive_start[basis][symbol] = n_uprimitive_;
            for (int i=0; i<shells.size(); ++i) {
                const ShellInfo &shell = shells[i];
                for(int prim = 0; prim < shell.nprimitive(); ++prim){
                    uexps.push_back(shell.exp(prim));
                    ucoefs.push_back(shell.coef(prim));
                    uoriginal_coefs.push_back(shell.original_coef(prim));
                    uerd_coefs.push_back(shell.erd_coef(prim));
                    n_uprimitive_++;
                }
            }
            primitive_end[basis][symbol] = n_uprimitive_;
        }
    }

    /*
     * Count basis functions, shells and primitives
     */
    n_uprimitive_ = 0;
    n_shells_ = 0;
    nprimitive_ = 0;
    nao_ = 0;
    nbf_ = 0;
    for (int n=0; n < natom; ++n) {
        const boost::shared_ptr<CoordEntry> &atom = molecule_->atom_entry(n);
        string basis = atom->basisset(basistype);
        string symbol = atom->symbol();
        vector<ShellInfo>& shells = shell_map[basis][symbol];
        for (int i=0; i<shells.size(); ++i) {
            const ShellInfo &shell = shells[i];
            int nprim = shell.nprimitive();
            n_uprimitive_ += nprim;
            nprimitive_ += nprim;
            n_shells_++;
            nao_ += shell.ncartesian();
            nbf_ += shell.nfunction();
        }
    }

    /*
     * Allocate arrays
     */
    n_prim_per_shell_ = new int[n_shells_];
    // The unique primitives
    uexponents_ = new double[n_uprimitive_];
    ucoefficients_ = new double[n_uprimitive_];
    uoriginal_coefficients_ = new double[n_uprimitive_];
    uerd_coefficients_ = new double[n_uprimitive_];
    for(int i = 0; i < n_uprimitive_; ++i){
        uexponents_[i] = uexps[i];
        ucoefficients_[i] = ucoefs[i];
        uoriginal_coefficients_[i] = uoriginal_coefs[i];
        uerd_coefficients_[i] = uerd_coefs[i];
    }

    shell_first_ao_ = new int[n_shells_];
    shell_first_basis_function_ = new int[n_shells_];
    shells_ = new GaussianShell[n_shells_];
    ao_to_shell_ = new int[nao_];
    function_to_shell_ = new int[nbf_];
    function_center_ = new int[nbf_];
    shell_center_ = new int[n_shells_];
    center_to_nshell_ = new int[natom];
    center_to_shell_ = new int[natom];
    xyz_ = new double[3*natom];

    /*
     * Now loop over all atoms, and point to the appropriate unique data
     */
    int shell_count = 0;
    int ao_count = 0;
    int bf_count = 0;
    double *xyz_ptr = xyz_;
    puream_ = false;
    max_am_ = 0;
    max_nprimitive_ = 0;
    for (int n = 0; n < natom; ++n) {
        const boost::shared_ptr<CoordEntry> &atom = molecule_->atom_entry(n);
        string basis = atom->basisset(basistype);
        string symbol = atom->symbol();
        vector<ShellInfo>& shells = shell_map[basis][symbol];
        int ustart = primitive_start[basis][symbol];
        int uend = primitive_end[basis][symbol];
        int nshells = shells.size();
        center_to_nshell_[n] = nshells;
        center_to_shell_[n] = shell_count;
        int atom_nprim = 0;
        for (int i = 0; i < nshells; ++i) {
            const ShellInfo &thisshell = shells[i];
            shell_first_ao_[shell_count] = ao_count;
            shell_first_basis_function_[shell_count] = bf_count;
            int shell_nprim = thisshell.nprimitive();
            int am = thisshell.am();
            max_nprimitive_ = shell_nprim > max_nprimitive_ ? shell_nprim : max_nprimitive_;
            max_am_ = max_am_ > am ? max_am_ : am;
            shell_center_[shell_count] = n;
            GaussianType puream = thisshell.is_pure() ? Pure : Cartesian;
            if(puream)
                puream_ = true;
//            fprintf(outfile, "atom %d basis %s shell %d nprim %d atom_nprim %d\n", n, basis.c_str(), i, shell_nprim, atom_nprim);
            shells_[shell_count] = GaussianShell(am, shell_nprim, &uoriginal_coefficients_[ustart+atom_nprim],
                    &ucoefficients_[ustart+atom_nprim], &uerd_coefficients_[ustart+atom_nprim], &uexponents_[ustart+atom_nprim], puream, n, xyz_ptr, bf_count);
            for(int thisbf = 0; thisbf < thisshell.nfunction(); ++thisbf){
                function_to_shell_[bf_count] = shell_count;
                function_center_[bf_count++] = n;
            }
            for(int thisao = 0; thisao < thisshell.ncartesian(); ++thisao){
                ao_to_shell_[ao_count++] = shell_count;
            }
            atom_nprim += shell_nprim;
            shell_count++;
        }
        Vector3 xyz = molecule_->xyz(n);
        xyz_ptr[0] = xyz[0];
        xyz_ptr[1] = xyz[1];
        xyz_ptr[2] = xyz[2];
        xyz_ptr += 3;
        if(atom_nprim != uend-ustart){
            throw PSIEXCEPTION("Problem with nprimitive in basis set construction!");
        }
    }
}

BasisSet::BasisSet(const BasisSet *bs, const int center)
{
    // Singletons; these should've been initialized by this point, but just in case
    if (initialized_shared_ == false)
        initialize_singletons();
    initialized_shared_ = true;

    /*
     * First, find the shells we need, and grab the data
     */
    std::vector<double> uexps;
    std::vector<double> ucoefs;
    std::vector<double> uoriginal_coefs;
    std::vector<double> uerd_coefs;
    name_ = bs->name();
    n_shells_ = 0;
    n_uprimitive_ = 0;
    nao_ = 0;
    nbf_ = 0;
    for(int shelln = 0; shelln < bs->nshell(); ++shelln){
        const GaussianShell &shell = bs->shell(shelln);
        if(shell.ncenter() == center){
            int nprim = shell.nprimitive();
            for(int prim = 0; prim < nprim; ++prim){
                uexps.push_back(shell.exp(prim));
                ucoefs.push_back(shell.coef(prim));
                uoriginal_coefs.push_back(shell.original_coef(prim));
                uerd_coefs.push_back(shell.erd_coef(prim));
                n_uprimitive_++;
            }
            n_shells_++;
            nao_ += shell.ncartesian();
            nbf_ += shell.nfunction();
        }
    }
    nprimitive_ = n_uprimitive_;


    // Create a "molecule", i.e., an atom
    boost::shared_ptr<Molecule> mol = bs->molecule();
    molecule_ = boost::shared_ptr<Molecule>(new Molecule);
    int Z = mol->Z(center);
    double x = mol->x(center);
    double y = mol->y(center);
    double z = mol->z(center);
    double mass = mol->mass(center);
    double charge = mol->charge(center);
    std::string lab = mol->label(center);
    char* label = new char[lab.length() + 1];
    strcpy(label,lab.c_str());
    //Put the atomic info into mol
    molecule_->add_atom(Z, 0.0, 0.0, 0.0, label, mass, charge);


    /*
     * Allocate arrays
     */
    n_prim_per_shell_ = new int[n_shells_];
    // The unique primitives
    uexponents_ = new double[n_uprimitive_];
    ucoefficients_ = new double[n_uprimitive_];
    uoriginal_coefficients_ = new double[n_uprimitive_];
    uerd_coefficients_ = new double[n_uprimitive_];
    for(int i = 0; i < n_uprimitive_; ++i){
        uexponents_[i] = uexps[i];
        ucoefficients_[i] = ucoefs[i];
        uoriginal_coefficients_[i] = uoriginal_coefs[i];
        uerd_coefficients_[i] = uoriginal_coefs[i];
    }

    shell_first_ao_ = new int[n_shells_];
    shell_first_basis_function_ = new int[n_shells_];
    shells_ = new GaussianShell[n_shells_];
    ao_to_shell_ = new int[nao_];
    function_to_shell_ = new int[nbf_];
    function_center_ = new int[nbf_];
    shell_center_ = new int[n_shells_];
    center_to_nshell_ = new int[1];
    center_to_shell_ = new int[1];
    xyz_ = new double[3];

    /*
     * Now loop over shell for this atom, and point to the appropriate unique data
     */
    int shell_count = 0;
    int ao_count = 0;
    int bf_count = 0;
    puream_ = false;
    max_am_ = 0;
    max_nprimitive_ = 0;
    int prim_count = 0;
    for(int shelln = 0; shelln < bs->nshell(); ++shelln){
        const GaussianShell &shell = bs->shell(shelln);
        if(shell.ncenter() == center){
            center_to_nshell_[0] = n_shells_;
            center_to_shell_[0] = shell_count;
            shell_first_ao_[shell_count] = ao_count;
            shell_first_basis_function_[shell_count] = bf_count;
            int shell_nprim = shell.nprimitive();
            int am = shell.am();
            max_nprimitive_ = shell_nprim > max_nprimitive_ ? shell_nprim : max_nprimitive_;
            max_am_ = max_am_ > am ? max_am_ : am;
            shell_center_[shell_count] = center;
            GaussianType puream = shell.is_pure() ? Pure : Cartesian;
            if(puream)
                puream_ = true;
            shells_[shell_count] = GaussianShell(am, shell_nprim, &uoriginal_coefficients_[prim_count],
                    &ucoefficients_[prim_count], &uerd_coefficients_[prim_count], &uexponents_[prim_count], puream, center, xyz_, bf_count);
            for(int thisbf = 0; thisbf < shell.nfunction(); ++thisbf){
                function_to_shell_[bf_count] = shell_count;
                function_center_[bf_count++] = center;
            }
            for(int thisao = 0; thisao < shell.ncartesian(); ++thisao){
                ao_to_shell_[ao_count++] = shell_count;
            }
            shell_count++;
            prim_count += shell_nprim;
        }
    }
    xyz_[0] = xyz_[1] = xyz_[2] = 0.0;
}

std::string BasisSet::make_filename(const std::string& name)
{
    // Modify the name of the basis set to generate a filename: STO-3G -> sto-3g
    string basisname = name;

    // First make it lower case
    transform(basisname.begin(), basisname.end(), basisname.begin(), ::tolower);

    string format_underscore("_"); // empty string
    // Replace all '(' with '_'
    xpressive::sregex match_format = xpressive::as_xpr("(");
    basisname = regex_replace(basisname, match_format, format_underscore);

    // Replace all ')' with '_'
    match_format = xpressive::as_xpr(")");
    basisname = regex_replace(basisname, match_format, format_underscore);

    // Replace all ',' with '_'
    match_format = xpressive::as_xpr(",");
    basisname = regex_replace(basisname, match_format, format_underscore);

    // Replace all '*' with 's'
    match_format = xpressive::as_xpr("*");
    string format_star("s");
    basisname = regex_replace(basisname, match_format, format_star);

    // Replace all '+' with 'p'
    match_format = xpressive::as_xpr("+");
    string format_plus("p");
    basisname = regex_replace(basisname, match_format, format_plus);

    // Add file extension
    basisname += ".gbs";

    return basisname;
}

void BasisSet::refresh()
{
    //TODO FIXME!!!
//    // Reset data to initial values
//    nprimitive_ = 0;
//    nao_ = 0;
//    nbf_ = 0;
//    max_am_ = 0;
//    max_nprimitive_ = 0;
//    puream_ = false;

//    shell_first_basis_function_.clear(); shell_first_basis_function_.resize(nshell(), 0);
//    shell_first_ao_.clear();             shell_first_ao_.resize(nshell(), 0);
//    shell_center_.clear();               shell_center_.resize(nshell(), 0);
//    function_center_.clear();
//    center_to_nshell_.clear();           center_to_nshell_.resize(molecule_->natom(), 0);
//    center_to_shell_.clear();            center_to_shell_.resize(molecule_->natom(), 0);
//    center_to_shell_[0] = 0;

//    int current_center = 0;

//    for (int i=0; i<nshell(); ++i) {
//        shell_center_[i]   = shells_[i].ncenter();
//        shell_first_ao_[i] = nao_;
//        shell_first_basis_function_[i] = nbf_;
//        shells_[i].set_function_index(nbf_);

//        center_to_nshell_[shell_center_[i]]++;
//        if (current_center != shell_center_[i]) {
//            center_to_shell_[shell_center_[i]] = i;
//            current_center = shell_center_[i];
//        }

//        nprimitive_ += shells_[i].nprimitive();
//        nao_        += shells_[i].ncartesian();
//        nbf_        += shells_[i].nfunction();

//        for (int m = 0; m < shells_[i].nfunction(); m++) {
//            function_center_.push_back(shells_[i].ncenter());
//        }

//        if (max_am_ < shells_[i].am())
//            max_am_ = shells_[i].am();

//        if (max_nprimitive_ < shells_[i].nprimitive())
//            max_nprimitive_ = shells_[i].nprimitive();

//        if (puream_ == false && shells_[i].is_pure())
//            puream_ = true;
//    }

//    function_to_shell_.resize(nbf());
//    int ifunc = 0;
//    for (int i=0; i<nshell(); ++i) {
//        int nfun = shells_[i].nfunction();
//        for (int j=0; j<nfun; ++j) {
//            function_to_shell_[ifunc] = i;
//            ifunc++;
//        }
//    }
//    ao_to_shell_.resize(nao());
//    ifunc = 0;
//    for (int i=0; i<nshell(); ++i) {
//        int nfun = shells_[i].ncartesian();
//        for (int j=0; j<nfun; ++j) {
//            ao_to_shell_[ifunc] = i;
//            ifunc++;
//        }
//    }

//    // Create a map that has a key/value pair
//    // The key is the angular momentum function of the shell arranged in decending order
//    // The value is the actual shell number
//    typedef std::pair<int, int> am_to_shell_pair;
//    std::multimap< int, int, std::less<int> > am_to_shell_list;
//    for (int i=0; i < shells_.size(); i++) {
//        am_to_shell_list.insert(am_to_shell_pair(shells_[i].nfunction(), i));
//    }
//    // This puts the sorted shell values into the sorted_shell_list_ vector
//    // This can be used by the integral iterator to look up the value of the sorted shells
//    std::multimap< int, int, std::less<int> >::iterator it;
//    sorted_ao_shell_list_.clear();
//    for (it=am_to_shell_list.begin(); it != am_to_shell_list.end(); it++) {
//        //std::cout << "sorted shell size = " << it->first <<
//        //        "\t, which belongs to shell number " << it->second << std::endl;
//        sorted_ao_shell_list_.push_back(it->second);
//    }
}

std::pair<std::vector<std::string>, boost::shared_ptr<BasisSet> > BasisSet::test_basis_set(int max_am)
{
#if 0
    int max_centers = 4;
    int max_primitives = 10;
    int max_shells;

    std::vector<int> nprim;
    nprim.push_back(10);
    nprim.push_back(1);
    nprim.push_back(6);
    nprim.push_back(1);
    nprim.push_back(2);
    nprim.push_back(1);
    nprim.push_back(1);
    nprim.push_back(1);
    nprim.push_back(1);
    nprim.push_back(1);

    std::vector<int> am;
    am.push_back(0);
    am.push_back(0);
    am.push_back(1);
    am.push_back(1);
    am.push_back(2);
    am.push_back(2);
    am.push_back(3);
    am.push_back(4);
    am.push_back(5);
    am.push_back(2);

    std::vector<std::vector<double> > c;
    c.push_back(std::vector<double>(10));
    c.push_back(std::vector<double>(1));
    c.push_back(std::vector<double>(6));
    c.push_back(std::vector<double>(1));
    c.push_back(std::vector<double>(2));
    c.push_back(std::vector<double>(1));
    c.push_back(std::vector<double>(1));
    c.push_back(std::vector<double>(1));
    c.push_back(std::vector<double>(1));
    c.push_back(std::vector<double>(1));

    std::vector<std::vector<double> > e;
    e.push_back(std::vector<double>(10));
    e.push_back(std::vector<double>(1));
    e.push_back(std::vector<double>(6));
    e.push_back(std::vector<double>(1));
    e.push_back(std::vector<double>(2));
    e.push_back(std::vector<double>(1));
    e.push_back(std::vector<double>(1));
    e.push_back(std::vector<double>(1));
    e.push_back(std::vector<double>(1));
    e.push_back(std::vector<double>(1));

    c[0][0] = 0.458878E-03;
    c[0][1] = 0.355070E-02;
    c[0][2] = 0.182618E-01;
    c[0][3] = 0.716650E-01;
    c[0][4] = 0.212346E+00;
    c[0][5] = 0.416203E+00;
    c[0][6] = 0.373020E+00;
    c[0][7] = 0.625054E-01;
    c[0][8] = 0.624532E-02;
    c[0][9] = 0.243374E-02;
    c[1][0] =     1.0;
    c[2][0] = 0.458878E-03;
    c[2][1] = 0.355070E-02;
    c[2][2] = 0.182618E-01;
    c[2][3] = 0.716650E-01;
    c[2][4] = 0.212346E+00;
    c[2][5] = 0.416203E+00;
    c[3][0] =     1.0;
    c[4][0] = 0.458878E-03;
    c[4][1] = 0.355070E-02;
    c[5][0] =     1.0;
    c[6][0] =     1.0;
    c[7][0] =     1.0;
    c[8][0] =     1.0;
    c[9][0] =     1.0;

    e[0][0] = 31700.0;
    e[0][1] =  4755.0;
    e[0][2] =  1082.0;
    e[0][3] =   306.0;
    e[0][4] =    99.0;
    e[0][5] =    33.0;
    e[0][6] =    13.0;
    e[0][7] =     4.0;
    e[0][8] =     2.0;
    e[0][9] =     0.5;
    e[1][0] =     1.0;
    e[2][0] = 31700.0;
    e[2][1] =  4755.0;
    e[2][2] =  1082.0;
    e[2][3] =   306.0;
    e[2][4] =    99.0;
    e[2][5] =    33.0;
    e[3][0] =     1.0;
    e[4][0] = 31700.0;
    e[4][1] =  4755.0;
    e[5][0] =     1.0;
    e[6][0] =     1.0;
    e[7][0] =     1.0;
    e[8][0] =     1.0;
    e[9][0] =     1.0;

    std::vector<std::string> labels;
    if (max_am > -1) {
        labels.push_back("S");
        labels.push_back("s");
        max_shells = 2;
    }
        labels.push_back("P");
    if (max_am > 0) {
        labels.push_back("p");
        max_shells = 4;
    }
    if (max_am > 1) {
        labels.push_back("D");
        labels.push_back("d");
        max_shells = 6;
    }
    if (max_am > 2) {
        labels.push_back("f");
        max_shells = 7;
    }
    if (max_am > 3) {
        labels.push_back("g");
        max_shells = 8;
    }
    if (max_am > 4) {
        labels.push_back("h");
        max_shells = 9;
    }
    if (max_am > 5) {
        labels.push_back("i");
        max_shells = 10;
    }

    boost::shared_ptr<BasisSet> new_basis(new BasisSet());

    // Add 4 atoms to the molecule for this basis (max integal centers is 4 at the moment)
    new_basis->molecule_ = boost::shared_ptr<Molecule>(new Molecule);
    // Ghost atoms are now handled differently, they are not added to the normal xyz information array,
    // but to the fxyz array.
    double x = 0.0;
    for (int A = 0; A < max_centers; A++) {
        new_basis->molecule_->add_atom(0, x, x, x);
        x += 1.0;
    }

    // Setup all the parameters needed for a zero basis set
    new_basis->shell_center_.resize(max_shells * max_centers);
    for (int A = 0; A < max_centers; A++)
        for (int Q = 0; Q < max_shells; Q++)
            new_basis->shell_center_[A*max_shells + Q] = A;
    new_basis->max_nprimitive_ = max_primitives;
    new_basis->max_am_ = max_am;

    // We'll time puream for now
    new_basis->puream_ = true;

    // Add shells
    for (int A = 0; A < max_centers; A++) {
        Vector3 center = new_basis->molecule_->fxyz(A);
        for (int Q = 0; Q < max_shells; Q++) {
            new_basis->shells_.push_back(GaussianShell(am[Q], c[Q], e[Q], Pure, A, center, 0));
        }
    }

    new_basis->refresh();

    return make_pair(labels, new_basis);
#endif
}


boost::shared_ptr<BasisSet> BasisSet::atomic_basis_set(int center)
{
    return boost::shared_ptr<BasisSet>(new BasisSet(this, center));
}

void BasisSet::compute_phi(double *phi_ao, double x, double y, double z)
{
  zero_arr(phi_ao, nao());

  int ao = 0;
  for(int ns=0; ns < nshell(); ns++) {
    const GaussianShell& shell = shells_[ns];
    int am = shell.am();
    int nprim = shell.nprimitive();
    const double *a = shell.exps();
    const double *c = shell.coefs();

    const double *xyz = shell.center();
    double dx = x - xyz[0];
    double dy = y - xyz[1];
    double dz = z - xyz[2];
    double rr = dx*dx + dy*dy + dz*dz;

    double cexpr = 0;
    for(int np=0; np < nprim; np++)
      cexpr += c[np] * exp(-a[np] * rr);

    for(int l=0; l < INT_NCART(am); l++) {
      Vector3& components = exp_ao[am][l];
      phi_ao[ao+l] += pow(dx, (double) components[0]) *
                      pow(dy, (double) components[1]) *
                      pow(dz, (double) components[2]) *
                      cexpr;
    }

      ao += INT_NCART(am);
  } // nshell
}

BasisSet BasisSet::concatenate(const BasisSet& b) const {

    BasisSet temp;
//TODO fixme!!!
#if 0
    temp.name_ = name_ + " + " + b.name_;
    temp.molecule_ = molecule();

    // Copy a's shells to temp
    temp.shells_ = shells_;

    // Append b's shells to temp
    temp.shells_.insert(temp.shells_.end(), b.shells_.begin(), b.shells_.end());

    // Call refresh to regenerate center_to_shell and center_to_nshell
    temp.refresh();
#endif
    return temp;
}

boost::shared_ptr<BasisSet> BasisSet::concatenate(const boost::shared_ptr<BasisSet>& b) const {
    return boost::shared_ptr<BasisSet>(new BasisSet(concatenate(*b.get())));
}

BasisSet BasisSet::add(const BasisSet& b) const {

    BasisSet temp;
//TODO fixme!!
#if 0
    temp.name_ = name_ + " + " + b.name_;
    temp.molecule_ = molecule();

    // Copy a's shells to temp
//    temp.shells_ = shells_;

    // Append b's shells to temp
//    std::vector<GaussianShell>::const_iterator iter = b.shells_.begin();
//    for (; iter != b.shells_.end(); iter++)
//        temp.shells_.push_back(*iter);

//    temp.shells_.insert(temp.shells_.end(), b.shells_.begin(), b.shells_.end());

    // Loop over atoms
    for (int atom=0; atom<molecule()->natom(); ++atom) {
        for (int shella=0; shella<nshell_on_center(atom); ++shella)
            temp.shells_.push_back(shell(atom, shella));

        for (int shellb=0; shellb<b.nshell_on_center(atom); ++shellb)
            temp.shells_.push_back(b.shell(atom, shellb));
    }

    // Call refresh to regenerate center_to_shell and center_to_nshell
    temp.refresh();

    // Sort by center number
//    std::sort(temp.shells_.begin(), temp.shells_.end(), shell_sorter_ncenter);

    // Call refresh to regenerate center_to_shell and center_to_nshell
//    temp.refresh();

    // Sort by AM in each center
//    for (int atom=0; atom < temp.molecule_->natom(); ++atom) {
//        std::sort(temp.shells_.begin()+temp.center_to_shell_[atom],
//                  temp.shells_.begin()+temp.center_to_shell_[atom]+temp.center_to_nshell_[atom],
//                  shell_sorter_am);
//    }
#endif
    return temp;
}

boost::shared_ptr<BasisSet> BasisSet::add(const boost::shared_ptr<BasisSet>& b) const {
    return boost::shared_ptr<BasisSet>(new BasisSet(add(*b.get())));
}


//boost::shared_ptr<BasisSet> BasisSet::concatenate(const boost::shared_ptr<BasisSet>& a, const boost::shared_ptr<BasisSet>& b) const {
//    return a->concatenate(b);
//}
