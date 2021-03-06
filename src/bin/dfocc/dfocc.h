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

#ifndef dfocc_h 
#define dfocc_h 

#include <libmints/wavefunction.h>
#include <libdiis/diismanager.h>
#include <psifiles.h>
#include "arrays.h"
#include "tensors.h"

using namespace std;

namespace psi{ 

namespace dfoccwave {

class DFOCC : public Wavefunction
{
 
    void common_init();
    
public:
    DFOCC(boost::shared_ptr<Wavefunction> reference_wavefunction, Options &options);

    virtual ~DFOCC();
    virtual bool same_a_b_orbs() const { return reference_wavefunction_->same_a_b_orbs(); }
    virtual bool same_a_b_dens() const { return reference_wavefunction_->same_a_b_dens(); }

    virtual double compute_energy();

protected:
    void mem_release();
    void get_moinfo();
    void title();
    void ref_energy();
    void mp2_energy();
    void scs_mp2_energy();
    void mp2_direct();
    void t1_1st_sc();
    void t2_1st_sc();
    void t2_1st_gen();
    void t2_1st_scs_sc();
    void t2_1st_scs_gen();
    void Fint_zero();
    void fock();
    void separable_tpdm();
    void gfock_oo();
    void gfock_vo();
    void gfock_ov();
    void gfock_vv();
    void gftilde_vv();
    void idp();
    void mograd();
    void occ_iterations();
    void kappa_orb_resp();
    void kappa_orb_resp_pcg();
    void orb_resp_pcg_rhf();
    void orb_resp_pcg_uhf();
    void kappa_diag_hess();
    void update_mo();
    void diis(int dimvec, SharedTensor2d &vecs, SharedTensor2d &errvecs, SharedTensor1d &vec_new, SharedTensor1d &errvec_new);
    void semi_canonic();
    void diagonal_mohess_vo();
    void diagonal_mohess_oo();
    void approx_diag_mohess_vo();
    void approx_diag_mohess_oo();
    void approx_diag_hf_mohess_vo();
    void approx_diag_hf_mohess_oo();
    void approx_diag_ekt_mohess_vo();
    void approx_diag_ekt_mohess_oo();
 
    // Conventional integrals for DF-BASIS-CC
    void tei_ijkl_chem();
    void tei_ijka_chem();
    void tei_ijab_chem();
    void tei_iajb_chem();
    void tei_oooo_chem();
    void tei_ooov_chem();
    void tei_oovv_chem();
    void tei_ovov_chem();

    void tei_ijkl_phys();
    void tei_ijka_phys();
    void tei_ijab_phys();
    void tei_iajb_phys();
    void tei_oooo_phys();
    void tei_ooov_phys();
    void tei_oovv_phys();
    void tei_ovov_phys();

    void tei_ijkl_anti_symm();
    void tei_ijka_anti_symm();
    void tei_ijab_anti_symm();
    void tei_iajb_anti_symm();
    void tei_oooo_anti_symm();
    void tei_ooov_anti_symm();
    void tei_oovv_anti_symm();
    void tei_ovov_anti_symm();

    // Conventional integrals for DF-BASIS-CC with direct algorithm
    // Integrals in chemist notations
    void tei_ijkl_chem_directAA(SharedTensor2d &K);
    void tei_ijkl_chem_directBB(SharedTensor2d &K);
    void tei_ijkl_chem_directAB(SharedTensor2d &K);

    void tei_ijka_chem_directAA(SharedTensor2d &K);
    void tei_ijka_chem_directBB(SharedTensor2d &K);
    void tei_ijka_chem_directAB(SharedTensor2d &K);
    void tei_iajk_chem_directAB(SharedTensor2d &K);

    void tei_ijab_chem_directAA(SharedTensor2d &K);
    void tei_ijab_chem_directBB(SharedTensor2d &K);
    void tei_ijab_chem_directAB(SharedTensor2d &K);
    void tei_abij_chem_directAB(SharedTensor2d &K);

    void tei_iajb_chem_directAA(SharedTensor2d &K);
    void tei_iajb_chem_directBB(SharedTensor2d &K);
    void tei_iajb_chem_directAB(SharedTensor2d &K);

    void tei_oooo_chem_directAA(SharedTensor2d &K);
    void tei_oooo_chem_directBB(SharedTensor2d &K);
    void tei_oooo_chem_directAB(SharedTensor2d &K);

    void tei_ooov_chem_directAA(SharedTensor2d &K);
    void tei_ooov_chem_directBB(SharedTensor2d &K);
    void tei_ooov_chem_directAB(SharedTensor2d &K);
    void tei_ovoo_chem_directAB(SharedTensor2d &K);

    void tei_oovv_chem_directAA(SharedTensor2d &K);
    void tei_oovv_chem_directBB(SharedTensor2d &K);
    void tei_oovv_chem_directAB(SharedTensor2d &K);
    void tei_vvoo_chem_directAB(SharedTensor2d &K);

    void tei_ovov_chem_directAA(SharedTensor2d &K);
    void tei_ovov_chem_directBB(SharedTensor2d &K);
    void tei_ovov_chem_directAB(SharedTensor2d &K);

    // Integrals in physist notations
    void tei_ijkl_phys_directAA(SharedTensor2d &K);
    void tei_ijkl_phys_directBB(SharedTensor2d &K);
    void tei_ijkl_phys_directAB(SharedTensor2d &K);

    void tei_ijka_phys_directAA(SharedTensor2d &K);
    void tei_ijka_phys_directBB(SharedTensor2d &K);
    void tei_ijka_phys_directAB(SharedTensor2d &K);
    void tei_ijak_phys_directAB(SharedTensor2d &K);

    void tei_ijab_phys_directAA(SharedTensor2d &K);
    void tei_ijab_phys_directBB(SharedTensor2d &K);
    void tei_ijab_phys_directAB(SharedTensor2d &K);

    void tei_iajb_phys_directAA(SharedTensor2d &K);
    void tei_iajb_phys_directBB(SharedTensor2d &K);
    void tei_iajb_phys_directAB(SharedTensor2d &K);
    void tei_aibj_phys_directAB(SharedTensor2d &K);

    void tei_oooo_phys_directAA(SharedTensor2d &K);
    void tei_oooo_phys_directBB(SharedTensor2d &K);
    void tei_oooo_phys_directAB(SharedTensor2d &K);

    void tei_ooov_phys_directAA(SharedTensor2d &K);
    void tei_ooov_phys_directBB(SharedTensor2d &K);
    void tei_ooov_phys_directAB(SharedTensor2d &K);
    void tei_oovo_phys_directAB(SharedTensor2d &K);

    void tei_oovv_phys_directAA(SharedTensor2d &K);
    void tei_oovv_phys_directBB(SharedTensor2d &K);
    void tei_oovv_phys_directAB(SharedTensor2d &K);

    void tei_ovov_phys_directAA(SharedTensor2d &K);
    void tei_ovov_phys_directBB(SharedTensor2d &K);
    void tei_ovov_phys_directAB(SharedTensor2d &K);
    void tei_vovo_phys_directAB(SharedTensor2d &K);

    // Anti-symmetrized integrals
    void tei_pqrs_anti_symm_direct(SharedTensor2d &K, SharedTensor2d &L);
    void tei_pqrs2_anti_symm_direct(SharedTensor2d &K, SharedTensor2d &L);
    void tei_pqrs3_anti_symm_direct(SharedTensor2d &K, SharedTensor2d &L, SharedTensor2d &M);

    // Conventional integrals for DF-BASIS-SCF
    void tei_oooo_chem_ref();
    void tei_ooov_chem_ref();
    void tei_oovv_chem_ref();
    void tei_ovov_chem_ref();

    void tei_oooo_phys_ref();
    void tei_ooov_phys_ref();
    void tei_oovv_phys_ref();
    void tei_ovov_phys_ref();

    void tei_oooo_anti_symm_ref();
    void tei_ooov_anti_symm_ref();
    void tei_oovv_anti_symm_ref();
    void tei_ovov_anti_symm_ref();

    // Conventional integrals for DF-BASIS-SCF with direct algorithm
    // Integrals in chemist notations
    void tei_oooo_chem_ref_directAA(SharedTensor2d &K);
    void tei_oooo_chem_ref_directBB(SharedTensor2d &K);
    void tei_oooo_chem_ref_directAB(SharedTensor2d &K);

    void tei_ooov_chem_ref_directAA(SharedTensor2d &K);
    void tei_ooov_chem_ref_directBB(SharedTensor2d &K);
    void tei_ooov_chem_ref_directAB(SharedTensor2d &K);
    void tei_ovoo_chem_ref_directAB(SharedTensor2d &K);

    void tei_oovv_chem_ref_directAA(SharedTensor2d &K);
    void tei_oovv_chem_ref_directBB(SharedTensor2d &K);
    void tei_oovv_chem_ref_directAB(SharedTensor2d &K);
    void tei_vvoo_chem_ref_directAB(SharedTensor2d &K);

    void tei_ovov_chem_ref_directAA(SharedTensor2d &K);
    void tei_ovov_chem_ref_directBB(SharedTensor2d &K);
    void tei_ovov_chem_ref_directAB(SharedTensor2d &K);

    void tei_vovo_chem_ref_directAA(SharedTensor2d &K);
    void tei_vovo_chem_ref_directBB(SharedTensor2d &K);
    void tei_vovo_chem_ref_directAB(SharedTensor2d &K);

    // Integrals in physist notations
    void tei_oooo_phys_ref_directAA(SharedTensor2d &K);
    void tei_oooo_phys_ref_directBB(SharedTensor2d &K);
    void tei_oooo_phys_ref_directAB(SharedTensor2d &K);

    void tei_ooov_phys_ref_directAA(SharedTensor2d &K);
    void tei_ooov_phys_ref_directBB(SharedTensor2d &K);
    void tei_ooov_phys_ref_directAB(SharedTensor2d &K);
    void tei_oovo_phys_ref_directAB(SharedTensor2d &K);

    void tei_oovv_phys_ref_directAA(SharedTensor2d &K);
    void tei_oovv_phys_ref_directBB(SharedTensor2d &K);
    void tei_oovv_phys_ref_directAB(SharedTensor2d &K);

    void tei_ovov_phys_ref_directAA(SharedTensor2d &K);
    void tei_ovov_phys_ref_directBB(SharedTensor2d &K);
    void tei_ovov_phys_ref_directAB(SharedTensor2d &K);
    void tei_vovo_phys_ref_directAB(SharedTensor2d &K);

    // df 
    void df();
    void df_corr();
    void df_ref();
    void trans_corr();
    void trans_ref();
    void trans_mp2();
    void formJ(boost::shared_ptr<BasisSet> auxiliary_, boost::shared_ptr<BasisSet> zero);
    void formJ_ref(boost::shared_ptr<BasisSet> auxiliary_, boost::shared_ptr<BasisSet> zero);
    void b_so(boost::shared_ptr<BasisSet> primary_, boost::shared_ptr<BasisSet> auxiliary_, boost::shared_ptr<BasisSet> zero);
    void b_so_ref(boost::shared_ptr<BasisSet> primary_, boost::shared_ptr<BasisSet> auxiliary_, boost::shared_ptr<BasisSet> zero);
    void b_oo();
    void b_ov();
    void b_vv();
    void b_ij();
    void b_ia();
    void b_ab();
    void c_oo();
    void c_ov();
    void c_vv();
    void c_ij();
    void c_ia();
    void c_ab();
    void b_oo_ref();
    void b_ov_ref();
    void b_vv_ref();
    void c_oo_ref();
    void c_ov_ref();
    void c_vv_ref();
    void trans_oei();
    void pair_index();
    void fock_so();
    void ref_grad();

    // Cholesky
    void cd_ints();
    void trans_cd();
    void trans_cd_mp2();
    void b_oo_cd();
    void b_ov_cd();
    void b_vv_cd();
    void b_ij_cd();
    void b_ia_cd();
    void b_ab_cd();

    // OMP2
    void omp2_manager();
    void mp2_manager();
    void cd_omp2_manager();
    void cd_mp2_manager();
    void omp2_opdm();
    void omp2_tpdm();
    void mp2l_energy();

    // OMP3
    void omp3_manager();
    void mp3_manager();

    // OMP2.5
    void omp2_5_manager();
    void mp2_5_manager();

    // OCEPA
    void ocepa_manager();
    void cepa_manager();

    // orbital pairs
    int so_pair_idx(int i, int j);
    int mo_pair_idx(int i, int j);
    int oo_pair_idxAA(int i, int j);
    int ij_pair_idxAA(int i, int j);
    int vv_pair_idxAA(int i, int j);
    int ab_pair_idxAA(int i, int j);
    int ov_pair_idxAA(int i, int j);
    int ia_pair_idxAA(int i, int j);
    int vo_pair_idxAA(int i, int j);
    int ai_pair_idxAA(int i, int j);
    int get_rotation_block(string rotblock);

    // DIIS
    DIISManager *t2DiisManager;
    
     int natom;
     int nmo;		// Number of MOs
     int nao;		// Number of AOs
     int nso;		// Number of SOs
     int noccA;		// Number of alpha occupied orbitals
     int noccB;		// Number of beta occupied orbitals
     int nvirA;		// Number of alpha virtual orbitals
     int nvirB;		// Number of beta virtual orbitals
     int naoccA;	// Number of active alpha occupied orbitals
     int naoccB;	// Number of active beta occupied orbitals
     int namo;		// Number of active SOs
     int navirA;	// Number of active alpha virtual orbitals
     int navirB;	// Number of active beta virtual orbitals
     int nirreps;	// Number of irreducible representations
     int nshell;	// Number of shells
     int nfrzc; 	// Number of frozen cores
     int nfrzv; 	// Number of frozen virtuals
     int npop; 		// Number of populated orbitals: npop=nmo-nfrzv
     int dimtei;	// dimension of tei in pitzer order for all integrals 
     int ntri; 		// square matrix dimension (nmo) -> pitzer order
     int ntri_so;	// square matrix dimension (nso) -> pitzer order
     int nQ;          // numer of aux-basis
     int nQ_ref;      // numer of aux-basis for DF_BASIS_SCF
     int nso2_;       // nso * nso
     int naocc2AA;     // # of active OO pairs 
     int naocc2AB;     // # of active OO pairs 
     int naocc2BB;     // # of active OO pairs 
     int navir2AA;     // # of active VV pairs 
     int navir2AB;     // # of active VV pairs 
     int navir2BB;     // # of active VV pairs 
     int navoAA;       // # of active VO pairs 
     int navoAB;       // # of active VO pairs 
     int navoBA;       // # of active VO pairs 
     int navoBB;       // # of active VO pairs 
     int nvoAA;        // # of all VO pairs 
     int nvoAB;        // # of all VO pairs 
     int nvoBA;        // # of all VO pairs 
     int nvoBB;        // # of all VO pairs 
     int nocc2AA;     // # of all OO pairs 
     int nocc2AB;     // # of all OO pairs 
     int nocc2BB;     // # of all OO pairs 
     int nvir2AA;     // # of all VV pairs 
     int nvir2AB;     // # of all VV pairs 
     int nvir2BB;     // # of all VV pairs 
     int nidpA;       // # of alpha independent pairs
     int nidpB;       // # of beta independent pairs
     int nidp;       
     int nidp_tot;       
     int idp_returnA;
     int idp_returnB;
     int nvar;
     int pcg_conver;

     int exp_cutoff;
     int multp; 
     int charge;
     int print_;
     int conver;
     int cc_maxiter;
     int mo_maxiter;
     int pcg_maxiter;
     int num_vecs;              // Number of vectors used in diis (diis order)
     int do_diis_;
     int itr_diis;
     int itr_occ;
     int itr_pcg;
     int time4grad;             // If 0 it is not the time for grad, if 1 it is the time for grad
     int cc_maxdiis_;           // MAX Number of vectors used in CC diis
     int cc_mindiis_;           // MIN Number of vectors used in CC diis
     int trans_ab;              // 0 means do not transform, 1 means do transform B(Q, ab)
     int mo_optimized;          // 0 means MOs are not optimized, 1 means Mos are optimized
     int orbs_already_opt;      // 0 false, 1 true
     int orbs_already_sc;       // 0 false, 1 true

     ULI memory;
     ULI memory_mb;
     ULI cost_ampAA;          // Mem required for the amplitudes
     ULI cost_ampBB;          // Mem required for the amplitudes
     ULI cost_ampAB;          // Mem required for the amplitudes
     ULI cost_amp;            // Mem required for the amplitudes
     ULI cost_df;             // Mem required for the df integrals

     // Common
     double Enuc;
     double Eelec;
     double Escf;
     double Eref;
     double Etotal;
     double Emp2;
     double Emp2_t1;
     double Emp2BB;
     double Emp2AA;
     double Emp2AB;
     double Emp2L;
     double Emp2L_old;
     double Ecorr;
     double EcorrL;
     double Ecc_rdm;
     double Escsmp2;
     double Escsmp2BB;
     double Escsmp2AA;
     double Escsmp2AB;
     double Esosmp2AB;
     double Esosmp2;
     double Escsnmp2;
     double Escsnmp2AA;
     double Escsnmp2BB;
     double Escsnmp2AB;
     double DE;
     double cutoff;
     double tol_Eod;
     double tol_grad;
     double tol_t2;
     double tol_pcg;
     double step_max;
     double mograd_max;
     double biggest_mograd;
     double biggest_mogradA;
     double biggest_mogradB;
     double biggest_kappa;
     double biggest_kappaA;
     double biggest_kappaB;
     double lshift_parameter;
     double os_scale;
     double ss_scale;
     double sos_scale;
     double sos_scale2;
     double e3_scale;
     double cepa_os_scale_;
     double cepa_ss_scale_;
     double cepa_sos_scale_;
     double sos_scale_ocepa;
     double rms_t2;
     double rms_t2AA;
     double rms_t2AB;
     double rms_t2BB;
     double rms_l2;
     double mu_ls;
     double sc_ls;
     double rms_wog;
     double rms_wogA;
     double rms_wogB;
     double rms_kappa;
     double rms_kappaA;
     double rms_kappaB;
     double msd_oo_scale;
     double reg_param;

     // OMP3
     double Emp3;
     double Emp3BB;
     double Emp3AA;
     double Emp3AB;
     double Emp3L;
     double Emp3L_old;
     double Escsmp3BB;
     double Escsmp3AA;
     double Escsmp3AB;
     double Escsmp3;
     double Esosmp3AB;
     double Esosmp3;

     // OCEPA
     double Ecepa;
     double Ecepa_old;
     double EcepaAA;
     double EcepaBB;
     double EcepaAB;
     double EcepaL;
     double EcepaL_old;
     double EscscepaBB;
     double EscscepaAA;
     double EscscepaAB;
     double Escscepa;
     double EsoscepaAB;
     double Esoscepa;
     
     string wfn;
     string reference;
     string reference_;
     string jobtype;
     string dertype;
     string basis;
     string level_shift;
     string lineq;
     string orth_type;
     string natorb;
     string semicanonic; 
     string opt_method; 
     string hess_type;
     string occ_orb_energy;
     string do_scs;		// Spin-Component-Scaling
     string do_sos;		// Spin-Opposite-Scaling
     string scs_type_;		
     string sos_type_;		
     string wfn_type_;
     string orb_resp_solver_;
     string pcg_beta_type_;
     string ekt_ip_;
     string ekt_ea_;
     string orb_opt_;
     string rotation_blocks;
     string conv_tei_type;
     string regularization; 
     string do_cd; 
     string read_scf_3index; 

     double **C_pitzerA;     
     double **C_pitzerB;     
     double **J_mhalf;     

     // Common
     SharedTensor2d CmoA;              // C(mu, p)
     SharedTensor2d CmoB;              // C(mu, p)
     SharedTensor2d Cmo_refA;          // Reference (initial) MOs
     SharedTensor2d Cmo_refB;          // Reference (initial) MOs
     SharedTensor2d CaoccA;            // C(mu, i) active
     SharedTensor2d CaoccB;            // C(mu, i) active
     SharedTensor2d CavirA;            // C(mu, a) active
     SharedTensor2d CavirB;            // C(mu, a) active
     SharedTensor2d CoccA;             // C(mu, i) all
     SharedTensor2d CoccB;             // C(mu, i) all
     SharedTensor2d CvirA;             // C(mu, a) all
     SharedTensor2d CvirB;             // C(mu, a) all
     SharedTensor2d HmoA;	 
     SharedTensor2d HmoB;	 
     SharedTensor2d FockA;	 
     SharedTensor2d FockB;	 
     SharedTensor2d Hso;
     SharedTensor2d Sso;
     SharedTensor2d Dso;
     SharedTensor2d DsoA;
     SharedTensor2d FsoA;
     SharedTensor2d FsoB;
     SharedTensor2d Wso;
     SharedTensor2d DQmatA;
     SharedTensor2d HooA;	 
     SharedTensor2d HooB;	 
     SharedTensor2d HovA;               
     SharedTensor2d HovB;               
     SharedTensor2d HvoA;	 
     SharedTensor2d HvoB;	 
     SharedTensor2d HvvA;	 
     SharedTensor2d HvvB;	 
     SharedTensor2d FooA;               
     SharedTensor2d FooB;               
     SharedTensor2d FovA;               
     SharedTensor2d FovB;               
     SharedTensor2d FvoA;               
     SharedTensor2d FvoB;               
     SharedTensor2d FvvA;               
     SharedTensor2d FvvB;               

     // DF Integrals
     SharedTensor2d Jmhalf;             // J Metric DF_BASIS_CC (RI)
     SharedTensor2d bQso;               // b(Q|mu nu) from DF_BASIS_CC (RI)
     SharedTensor2d bQnoA;              // b(Q|mu i)
     SharedTensor2d bQnoB;              // b(Q|mu i)
     SharedTensor2d bQnvA;              // b(Q|mu a)
     SharedTensor2d bQnvB;              // b(Q|mu a)
     SharedTensor2d bQooA;              // b(Q|i j) : all
     SharedTensor2d bQooB;              // b(Q|i j) : all
     SharedTensor2d bQovA;              // b(Q|i a) : all
     SharedTensor2d bQovB;              // b(Q|i a) : all
     SharedTensor2d bQvvA;              // b(Q|a b) : all
     SharedTensor2d bQvvB;              // b(Q|a b) : all
     SharedTensor2d bQijA;              // b(Q|i j) : active
     SharedTensor2d bQijB;              // b(Q|i j) : active
     SharedTensor2d bQiaA;              // b(Q|i a) : active
     SharedTensor2d bQiaB;              // b(Q|i a) : active
     SharedTensor2d bQabA;              // b(Q|a b) : active
     SharedTensor2d bQabB;              // b(Q|a b) : active

     SharedTensor2d cQso;               // c(Q|mu nu) from DF_BASIS_CC (RI)
     SharedTensor2d cQnoA;              // c(Q|mu i)
     SharedTensor2d cQnoB;              // c(Q|mu i)
     SharedTensor2d cQnvA;              // c(Q|mu a)
     SharedTensor2d cQnvB;              // c(Q|mu a)
     SharedTensor2d cQooA;              // c(Q|i j) : all
     SharedTensor2d cQooB;              // c(Q|i j) : all
     SharedTensor2d cQovA;              // c(Q|i a) : all
     SharedTensor2d cQovB;              // c(Q|i a) : all
     SharedTensor2d cQvvA;              // c(Q|a b) : all
     SharedTensor2d cQvvB;              // c(Q|a b) : all
     SharedTensor2d cQijA;              // c(Q|i j) : active
     SharedTensor2d cQijB;              // c(Q|i j) : active
     SharedTensor2d cQiaA;              // c(Q|i a) : active
     SharedTensor2d cQiaB;              // c(Q|i a) : active
     SharedTensor2d cQabA;              // c(Q|a b) : active
     SharedTensor2d cQabB;              // c(Q|a b) : active

     // DF OPDM
     SharedTensor2d G1c_ij;             
     SharedTensor2d G1c_ab;             
     SharedTensor2d G1c_oo;             
     SharedTensor2d G1c_vv;             
     SharedTensor2d G1c;               // Correlation part of OPDM
     SharedTensor2d G1;                // Full OPDM
     SharedTensor2d G1c_ijA;             
     SharedTensor2d G1c_ijB;             
     SharedTensor2d G1c_abA;             
     SharedTensor2d G1c_abB;             
     SharedTensor2d G1c_ooA;             
     SharedTensor2d G1c_ooB;             
     SharedTensor2d G1c_vvA;             
     SharedTensor2d G1c_vvB;             
     SharedTensor2d G1cA;              // Correlation part of OPDM
     SharedTensor2d G1cB;              // Correlation part of OPDM
     SharedTensor2d G1A;                // Full OPDM
     SharedTensor2d G1B;                // Full OPDM
     SharedTensor2d GijA;              
     SharedTensor2d GijB;              
     SharedTensor2d GabA;              
     SharedTensor2d GabB;              

     // DF TPDM
     SharedTensor2d G2c_ij;                                    
     SharedTensor2d G2c_ia;                                    
     SharedTensor2d G2c_ab;
     SharedTensor2d G2c_oo;                                    
     SharedTensor2d G2c_ov;                                    
     SharedTensor2d G2c_vo;                                    
     SharedTensor2d G2c_vv;
     SharedTensor2d G2c;               // Correlation part of TPDM
     SharedTensor2d G2c_ijA;
     SharedTensor2d G2c_ijB;
     SharedTensor2d G2c_iaA;                                    
     SharedTensor2d G2c_iaB;                                    
     SharedTensor2d G2c_abA;
     SharedTensor2d G2c_abB;
     SharedTensor2d G2c_ooA;                                    
     SharedTensor2d G2c_ooB;                                    
     SharedTensor2d G2c_ovA;                                    
     SharedTensor2d G2c_ovB;                                    
     SharedTensor2d G2c_voA;                                    
     SharedTensor2d G2c_voB;                                    
     SharedTensor2d G2c_vvA;
     SharedTensor2d G2c_vvB;
     SharedTensor2d G2cA;              // Correlation part of TPDM
     SharedTensor2d G2cB;              // Correlation part of TPDM
     SharedTensor1d Jc;                // Correlation Coulomb matrix
     SharedTensor1d g1Q;              
     SharedTensor1d g1Qc;              
     SharedTensor1d g1Qt;              
     SharedTensor1d g1Qt2;              

     // DF GFM
     SharedTensor2d GF;                // Full GFM
     SharedTensor2d GFA;               // Full GFM
     SharedTensor2d GFB;               // Full GFM
     SharedTensor2d GFoo;             
     SharedTensor2d GFov;             
     SharedTensor2d GFvo;             
     SharedTensor2d GFvv;             
     SharedTensor2d GFooA;
     SharedTensor2d GFooB;
     SharedTensor2d GFovA;             
     SharedTensor2d GFovB;             
     SharedTensor2d GFvoA;             
     SharedTensor2d GFvoB;             
     SharedTensor2d GFvvA;
     SharedTensor2d GFvvB;

     SharedTensor2d GFtvv;            // Complement of GFM 
     SharedTensor2d GFtvvA;           // Complement of GFM 
     SharedTensor2d GFtvvB;           // Complement of GFM 

     SharedTensor2d GFc;              // Correlation GFM
     SharedTensor2d GFcA;             // Correlation GFM
     SharedTensor2d GFcB;             // Correlation GFM
     SharedTensor2d GFc_oo;             
     SharedTensor2d GFc_ov;             
     SharedTensor2d GFc_vo;             
     SharedTensor2d GFc_vv;             
     SharedTensor2d GFc_ooA;             
     SharedTensor2d GFc_ooB;             
     SharedTensor2d GFc_ovA;             
     SharedTensor2d GFc_ovB;             
     SharedTensor2d GFc_voA;             
     SharedTensor2d GFc_voB;             
     SharedTensor2d GFc_vvA;             
     SharedTensor2d GFc_vvB;             

     SharedTensor2d GFs;              // Separable part of GFM
     SharedTensor2d GFsA;             // Separable part of GFM
     SharedTensor2d GFsB;             // Separable part of GFM
     SharedTensor2d GFs_oo;                                               
     SharedTensor2d GFs_ov;
     SharedTensor2d GFs_vo;
     SharedTensor2d GFs_vv;
     SharedTensor2d GFs_ooA;
     SharedTensor2d GFs_ooB;
     SharedTensor2d GFs_ovA;
     SharedTensor2d GFs_ovB;
     SharedTensor2d GFs_voA;
     SharedTensor2d GFs_voB;
     SharedTensor2d GFs_vvA;
     SharedTensor2d GFs_vvB;

     // MO gradient and Hessian
     SharedTensor2d Worb;              // MO gradient matrix
     SharedTensor2d WorbA;
     SharedTensor2d WorbB;
     SharedTensor2d Aorb;              // MO Hessian matrix
     SharedTensor2d AorbAA;
     SharedTensor2d AorbAB;
     SharedTensor2d AorbBB;
     SharedTensor2d Aaibj;
     SharedTensor2d Aaijk;
     SharedTensor2d Aijkl;
     SharedTensor2d Aoo;
     SharedTensor2d Avo;
     SharedTensor2d AooA;
     SharedTensor2d AooB;
     SharedTensor2d AvoA;
     SharedTensor2d AvoB;

     // Orbital rotations
     SharedTensor2d UorbA;           // MO rotation matrix: wrt reference MOs
     SharedTensor2d UorbB;
     SharedTensor2d KorbA;           // K matrix: wrt reference MOs 
     SharedTensor2d KorbB;
     SharedTensor2d KsqrA;
     SharedTensor2d KsqrB;
 
     // MO rotation vectors
     SharedTensor1d wog;             // MO gradient vector
     SharedTensor1d wogA;            // MO gradient vector
     SharedTensor1d wogB;
     SharedTensor1d wog_intA;        // Interpolated MO gradient vector 
     SharedTensor1d wog_intB;
     SharedTensor1d kappa;           // where kappa = kappaA + kappaB
     SharedTensor1d kappaA;          // vector of orb rot parameters: wrt previous MOS
     SharedTensor1d kappaB;
     SharedTensor1d kappa_barA;      // vector of orb rot parameters: wrt reference MOS
     SharedTensor1d kappa_barB;
     SharedTensor1d kappa_newA;
     SharedTensor1d kappa_newB;
     SharedTensor1d zvector;
     SharedTensor1d zvectorA;
     SharedTensor1d zvectorB;
     SharedTensor1d zvec_newA;
     SharedTensor1d zvec_newB;

     // PCG intermediates
     SharedTensor1d r_pcgA;
     SharedTensor1d r_pcgB;
     SharedTensor1d z_pcgA;
     SharedTensor1d z_pcgB;
     SharedTensor1d p_pcgA;
     SharedTensor1d p_pcgB;
     SharedTensor1d sigma_pcgA;
     SharedTensor1d sigma_pcgB;
     SharedTensor1d Minv_pcgA;
     SharedTensor1d Minv_pcgB;
     SharedTensor1d r_pcg_newA;
     SharedTensor1d r_pcg_newB;
     SharedTensor1d z_pcg_newA;
     SharedTensor1d z_pcg_newB;
     SharedTensor1d p_pcg_newA;
     SharedTensor1d p_pcg_newB;
     SharedTensor1d dr_pcgA;
     SharedTensor1d dr_pcgB;

     // Independent pairs
     SharedTensor1i idprowA;
     SharedTensor1i idprowB;
     SharedTensor1i idpcolA;
     SharedTensor1i idpcolB;

     // Diis
     SharedTensor2d vecsA;
     SharedTensor2d vecsB;
     SharedTensor2d errvecsA;
     SharedTensor2d errvecsB;
  
     // SO basis
     SharedTensor2d gQso;              // Gamma(Q|mu nu): 3-index TPDM 
     SharedTensor2d gQso_ref;          // Gamma(Q|mu nu): 3-index TPDM 
     SharedTensor2d gQoo;              // Gamma(Q|i i): 3-index TPDM 
     SharedTensor2d gQoo_ref;          // Gamma(Q|i i): 3-index TPDM 
     SharedTensor2d gQon_ref;          // Gamma(Q|i nu): 3-index TPDM 
     SharedTensor2d Gaux;              // Gamma(P,Q): 2-index TPDM 
     SharedTensor2d Gaux_ref;          // Gamma(P,Q): 2-index TPDM 
     SharedTensor2d dQso;              // D(Q|mu nu): 3-index OPDM for REF WFN 

     // Amplitudes
     SharedTensor2d t2_1;              // T_ij^ab(1)
     SharedTensor2d t2p_1;             // T'(ia,jb) = T_ij^ab(1)
     SharedTensor2d u2_1;              // 2*T_ij^ab(1) - T_ji^ab(1)
     SharedTensor2d u2p_1;             // U'(ia,jb) = 2*T_ij^ab(1) - T_ji^ab(1)
     SharedTensor2d t2p_1new;          // T'(ia,jb) = T_ij^ab(1)

     SharedTensor2d t2_1AA;            // T_ij^ab(1)
     SharedTensor2d t2_1AB;            // T_ij^ab(1)
     SharedTensor2d t2_1BB;            // T_ij^ab(1)
     SharedTensor2d t2_1newAA;         // T_ij^ab(1)
     SharedTensor2d t2_1newAB;         // T_ij^ab(1)
     SharedTensor2d t2_1newBB;         // T_ij^ab(1)

     SharedTensor2d t1A;               // T_i^a(1)
     SharedTensor2d t1B;               // T_i^a(1)

     SharedTensor2d FijA;               
     SharedTensor2d FijB;               
     SharedTensor2d FabA;               
     SharedTensor2d FabB;               

     // Intermediates
     SharedTensor2d uQia;              

     // Conventional integrals for the DF_BASIS_CC
     SharedTensor2d JijklAA;             // (ij|kl) (active)
     SharedTensor2d JijklAB;             // (ij|kl) (active)
     SharedTensor2d JijklBB;             // (ij|kl) (active)
     SharedTensor2d JijkaAA;             // (ij|ka) (active)
     SharedTensor2d JijkaAB;             // (ij|ka) (active)
     SharedTensor2d JijkaBB;             // (ij|ka) (active)
     SharedTensor2d JijabAA;             // (ij|ab) (active)
     SharedTensor2d JijabAB;             // (ij|ab) (active)
     SharedTensor2d JijabBB;             // (ij|ab) (active)
     SharedTensor2d JiajbAA;             // (ia|jb) (active)
     SharedTensor2d JiajbAB;             // (ia|jb) (active)
     SharedTensor2d JiajbBB;             // (ia|jb) (active)
     SharedTensor2d JiajkAB;             // (ka|ij) (active)
     SharedTensor2d JabijAB;             // (ab|ij) (active)

     SharedTensor2d JooooAA;             // (ij|kl) (all)
     SharedTensor2d JooooAB;             // (ij|kl) (all)
     SharedTensor2d JooooBB;             // (ij|kl) (all)
     SharedTensor2d JooovAA;             // (ij|ka) (all)
     SharedTensor2d JooovAB;             // (ij|ka) (all)
     SharedTensor2d JooovBB;             // (ij|ka) (all)
     SharedTensor2d JoovvAA;             // (ij|ab) (all)
     SharedTensor2d JoovvAB;             // (ij|ab) (all)
     SharedTensor2d JoovvBB;             // (ij|ab) (all)
     SharedTensor2d JovovAA;             // (ia|jb) (all)
     SharedTensor2d JovovBB;             // (ia|jb) (all)
     SharedTensor2d JovovAB;             // (ia|jb) (all)
     SharedTensor2d JovooAB;             // (ka|ij) (all)
     SharedTensor2d JvvooAB;             // (ab|ij) (all)

     SharedTensor2d IijklAA;             // <ij|kl> (active)
     SharedTensor2d IijklAB;             // <ij|kl> (active)
     SharedTensor2d IijklBB;             // <ij|kl> (active)
     SharedTensor2d IijkaAA;             // <ij|ka> (active)
     SharedTensor2d IijkaAB;             // <ij|ka> (active)
     SharedTensor2d IijkaBB;             // <ij|ka> (active)
     SharedTensor2d IijabAA;             // <ij|ab> (active)
     SharedTensor2d IijabAB;             // <ij|ab> (active)
     SharedTensor2d IijabBB;             // <ij|ab> (active)
     SharedTensor2d IiajbAA;             // <ia|jb> (active)
     SharedTensor2d IiajbAB;             // <ia|jb> (active)
     SharedTensor2d IiajbBB;             // <ia|jb> (active)
     SharedTensor2d IijakAB;             // <ij|ak> (active)
     SharedTensor2d IaibjAB;             // <ai|bj> (active)

     SharedTensor2d IooooAA;             // <ij|kl> (all)
     SharedTensor2d IooooAB;             // <ij|kl> (all)
     SharedTensor2d IooooBB;             // <ij|kl> (all)
     SharedTensor2d IooovAA;             // <ij|ka> (all)
     SharedTensor2d IooovAB;             // <ij|ka> (all)
     SharedTensor2d IooovBB;             // <ij|ka> (all)
     SharedTensor2d IoovvAA;             // <ij|ab> (all)
     SharedTensor2d IoovvAB;             // <ij|ab> (all)
     SharedTensor2d IoovvBB;             // <ij|ab> (all)
     SharedTensor2d IovovAA;             // <ia|jb> (all)
     SharedTensor2d IovovAB;             // <ia|jb> (all)
     SharedTensor2d IovovBB;             // <ia|jb> (all)
     SharedTensor2d IoovoAB;             // <ij|ak> (all)
     SharedTensor2d IvovoAB;             // <ai|bj> (all)

     SharedTensor2d AIijklAA;             // <ij||kl> (active)
     SharedTensor2d AIijklBB;             // <ij||kl> (active)
     SharedTensor2d AIijkaAA;             // <ij||ka> (active)
     SharedTensor2d AIijkaBB;             // <ij||ka> (active)
     SharedTensor2d AIijabAA;             // <ij||ab> (active)
     SharedTensor2d AIijabBB;             // <ij||ab> (active)
     SharedTensor2d AIiajbAA;             // <ia||jb> (active)
     SharedTensor2d AIiajbBB;             // <ia||jb> (active)

     SharedTensor2d AIooooAA;             // <ij||kl> (all)
     SharedTensor2d AIooooBB;             // <ij||kl> (all)
     SharedTensor2d AIooovAA;             // <ij||ka> (all)
     SharedTensor2d AIooovBB;             // <ij||ka> (all)
     SharedTensor2d AIoovvAA;             // <ij||ab> (all)
     SharedTensor2d AIoovvBB;             // <ij||ab> (all)
     SharedTensor2d AIovovAA;             // <ia||jb> (all)
     SharedTensor2d AIovovBB;             // <ia||jb> (all)

     // 1D-Tensors
     SharedTensor1d DQvecA;
     SharedTensor1d dQ;

     // Pair indices
     SharedTensor2i so_idx;             // Pair index for active SO 
     SharedTensor2i ij_idxAA;           // Pair index for active OO 
     SharedTensor2i ij_idxAB;           // Pair index for active OO 
     SharedTensor2i ij_idxBA;           // Pair index for active OO 
     SharedTensor2i ij_idxBB;           // Pair index for active OO 
     SharedTensor2i ia_idxAA;           // Pair index for active OV 
     SharedTensor2i ia_idxAB;           // Pair index for active OV 
     SharedTensor2i ia_idxBA;           // Pair index for active OV 
     SharedTensor2i ia_idxBB;           // Pair index for active OV 
     SharedTensor2i ai_idxAA;           // Pair index for active VO 
     SharedTensor2i ai_idxAB;           // Pair index for active VO 
     SharedTensor2i ai_idxBA;           // Pair index for active VO 
     SharedTensor2i ai_idxBB;           // Pair index for active VO 
     SharedTensor2i ab_idxAA;           // Pair index for active VV 
     SharedTensor2i ab_idxAB;           // Pair index for active VV 
     SharedTensor2i ab_idxBA;           // Pair index for active VV 
     SharedTensor2i ab_idxBB;           // Pair index for active VV 

     SharedTensor2i oo_idxAA;           // Pair index for all OO 
     SharedTensor2i oo_idxAB;           // Pair index for all OO 
     SharedTensor2i oo_idxBB;           // Pair index for all OO 
     SharedTensor2i ov_idxAA;           // Pair index for all OV 
     SharedTensor2i ov_idxAB;           // Pair index for all OV 
     SharedTensor2i ov_idxBB;           // Pair index for all OV 
     SharedTensor2i vo_idxAA;           // Pair index for all VO 
     SharedTensor2i vo_idxAB;           // Pair index for all VO 
     SharedTensor2i vo_idxBB;           // Pair index for all VO 
     SharedTensor2i vv_idxAA;           // Pair index for all VV 
     SharedTensor2i vv_idxAB;           // Pair index for all VV 
     SharedTensor2i vv_idxBB;           // Pair index for all VV 

     SharedMatrix Tso_;
     SharedMatrix Vso_;
     SharedMatrix Hso_;
     SharedMatrix Sso_;
     SharedMatrix bQnn;         // b(Q|mu nu)
     SharedVector e_orbA;
    
};

} }

#endif // dfocc_h

