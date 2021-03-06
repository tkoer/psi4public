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

#ifndef ASAPT_VIS_H
#define ASAPT_VIS_H

#include <libmints/typedefs.h>
#include <libmints/wavefunction.h>
#include <map>
#include <set>

namespace psi {

class Options;
class BasisExtents;
class RKSFunctions;
class BlockOPoints;
class AtomicDensity;

namespace dftsapt {

/* ASAPT is required to provide the following partitions in vars:
 *
 *  Elst_AB
 *  Exch_ab
 *  IndAB_aB
 *  IndBA_Ab
 *  Disp_ab
 *
 * ASAPTVis will handle the rest, upon a call to analyze()
 *
 * Relevant analysis tasks include (in tasks):
 *  Atomic1   - Standard order-1 atomic partition
 *  Atomic2   - Standard order-2 atomic partition
 *  Orbital1  - Standard order-1 orbital partition
 *  Orbital2  - Standard order-2 orbital partition
 *  Voxel     - Voxel-based visualization, using grid parameters in Options
 *   
 */

class ASAPTVis {

protected:

    /// Options object (provides task list and extensive voxel parameters) 
    Options& options_;
    /// Primary basis set
    boost::shared_ptr<BasisSet> primary_;
    /// Molecular geometry of monomer A
    boost::shared_ptr<Molecule> monomer_A_;
    /// Molecular geometry of monomer B
    boost::shared_ptr<Molecule> monomer_B_;
    /// Local orbitals for A
    boost::shared_ptr<Matrix> Locc_A_;
    /// Local orbitals for B
    boost::shared_ptr<Matrix> Locc_B_;
    /// Local orbital charges for A
    boost::shared_ptr<Matrix> Q_A_;
    /// Local orbital charges for B
    boost::shared_ptr<Matrix> Q_B_;
    /// Atomic densities for A
    boost::shared_ptr<AtomicDensity> atomic_A_;
    /// Atomic densities for B 
    boost::shared_ptr<AtomicDensity> atomic_B_; 

    /// Map of Component partitions
    std::map<std::string, boost::shared_ptr<Matrix> > vars_;
    /// List of required tasks
    std::set<std::string> tasks_;

    /// Compute all relevant partial sums in vars
    void summations();
    /// Drop standard atomic traces to disk
    void drop_atomic_1();
    /// Drop standard atomic pairs to disk
    void drop_atomic_2();
    /// Drop standard orbital traces to disk
    void drop_orbital_1();
    /// Drop standard orbital pairs to disk
    void drop_orbital_2();
    /// Drop the voxel analysis to disk
    void drop_voxel();
    /// Drop the debug voxel analysis to disk (orbitals and atoms)
    void drop_debug();
    /// Drop a matrix in vars to disk
    void drop(const std::string& val);
    
public:
    ASAPTVis(
        boost::shared_ptr<BasisSet> primary, 
        boost::shared_ptr<Molecule> monomer_A,
        boost::shared_ptr<Molecule> monomer_B,
        boost::shared_ptr<Matrix> Locc_A, 
        boost::shared_ptr<Matrix> Locc_B, 
        boost::shared_ptr<Matrix> Q_A, 
        boost::shared_ptr<Matrix> Q_B,
        boost::shared_ptr<AtomicDensity> atomic_A,
        boost::shared_ptr<AtomicDensity> atomic_B
        );
    virtual ~ASAPTVis();

    std::map<std::string, boost::shared_ptr<Matrix> >& vars() { return vars_; }    

    void analyze();
};

class CubicDensityGrid {

protected:
    
    /// Options object for overages and voxel spacing
    Options& options_;
    /// Molecule this grid is built around
    boost::shared_ptr<Molecule> mol_;
    /// Basis set this grid is built around, for sparsity considerations
    boost::shared_ptr<BasisSet> primary_;

    // => Physical Grid <= //

    // Voxel quanta in x, y, z [(N_x+1) x (N_y + 1) x (N_z + 1)]
    int N_[3];
    // Voxel spacing in x, y, z
    double D_[3];
    // Voxel origin in x, y, z
    double O_[3];

    /// number of points of grid
    size_t npoints_;
    /// blocking in all cardinal directions
    size_t nxyz_;
    /// x coordinates of grid
    double* x_;
    /// y coordinates of grid
    double* y_;
    /// z coordinates of grid
    double* z_;
    
    /// Vector of blocks 
    std::vector<boost::shared_ptr<BlockOPoints> > blocks_;
    /// Points to basis extents, built internally
    boost::shared_ptr<BasisExtents> extents_;
    // RKS points object
    boost::shared_ptr<RKSFunctions> points_;

    // => Stuff on this Grid <= //

    // Value of trace of D on the grid
    double* v_;

public:
    CubicDensityGrid(
        boost::shared_ptr<BasisSet> primary);
    virtual ~CubicDensityGrid(); 

    void build_grid();
    void print_header();
    
    // => Production-Level Visualization <= //

    // Zero the grid property
    void zero();
    // Compute a generalized electronic density on this grid
    void compute_electronic(boost::shared_ptr<Matrix> D);
    // Compute an atomic-density weighted value on the grid
    void compute_atomic(boost::shared_ptr<Vector> V, boost::shared_ptr<AtomicDensity> atomic);
    // Drop a raw float32 variant of this dataset to disk (uses internal v_ if v == NULL) 
    void drop_raw(const std::string& file, double clamp, double* v = NULL);
    // Drop a raw UVF variant of this dataset to disk (uses internal v_ if v == NULL) 
    void drop_uvf(const std::string& file, double clamp, double* v = NULL);

    // => Debugging-Level Visualization <= //

    // Drop all atomic densities to raw files, using labeling [label "qN.raw"] and [label "wN.raw"]
    void compute_atomic_densities(boost::shared_ptr<AtomicDensity> atomic, double clamp, const std::string& label);
    // Drop all orbitals to raw files, using labeling [label "fN.raw"]
    void compute_orbitals(boost::shared_ptr<Matrix> Cocc, double clamp, const std::string& label);
};

}} // End namespace

#endif

