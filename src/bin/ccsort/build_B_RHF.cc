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

/*! \file
    \ingroup CCSORT
    \brief Enter brief description of file here 
*/
#include <libdpd/dpd.h>
#include <psifiles.h>
#include "MOInfo.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace ccsort {

/* build_B_RHF_singlet(): Builds the RHF orbital Hessian for computing
** spatial- and spin-symmetry instabilities.  In spin orbitals the Hessian
** matrix is:
**
** B(ai,bj) = delta_ij f_ab - delta_ab f_ij + <ij||ab> - <ja||ib>
**
** RHF references and singlet eigenstates:
**  B(AI,BJ) = delta_IJ f_AB - delta_AB f_IJ + <IJ|BA> - <IA|JB>
**
** This routine will build each spin component of the entire matrix 
** for later in-core diagonalization.
**
** TDC, March 2003
**
** Adapted for use in local-CC response calculations by TDC, April
** 2004.  Note that this version of the MO Hessian is correct for
** non-canonical Hartree-Fock orbitals.
*/

void build_B_RHF(double omega)
{
  int h, nirreps;
  int a, b, i, j, ai, bj, A, B, I, J, Asym, Bsym, Isym, Jsym;
  dpdbuf4 C, D, Bmat;
  dpdfile2 fIJ, fij, fAB, fab;
	
  nirreps = moinfo.nirreps;
	
  psio_open(PSIF_MO_HESS, 0);
	
  global_dpd_->buf4_init(&D, PSIF_CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  global_dpd_->buf4_sort(&D, PSIF_MO_HESS, sprq, 11, 11, "B(AI,BJ)");
  global_dpd_->buf4_close(&D);
	
  global_dpd_->buf4_init(&C, PSIF_CC_CINTS, 0, 10, 10, 10, 10, 0, "C <ia|jb>");
  global_dpd_->buf4_sort_axpy(&C, PSIF_MO_HESS, qpsr, 11, 11, "B(AI,BJ)", -1);
  global_dpd_->buf4_close(&C);
	
  global_dpd_->file2_init(&fIJ, PSIF_CC_OEI, 0, 0, 0, "fIJ");
  global_dpd_->file2_mat_init(&fIJ);
  global_dpd_->file2_mat_rd(&fIJ);
  global_dpd_->file2_init(&fij, PSIF_CC_OEI, 0, 0, 0, "fij");
  global_dpd_->file2_mat_init(&fij);
  global_dpd_->file2_mat_rd(&fij);
  global_dpd_->file2_init(&fAB, PSIF_CC_OEI, 0, 1, 1, "fAB");
  global_dpd_->file2_mat_init(&fAB);
  global_dpd_->file2_mat_rd(&fAB);
  global_dpd_->file2_init(&fab, PSIF_CC_OEI, 0, 1, 1, "fab");
  global_dpd_->file2_mat_init(&fab);
  global_dpd_->file2_mat_rd(&fab);
	
  global_dpd_->buf4_init(&Bmat, PSIF_MO_HESS, 0, 11, 11, 11, 11, 0, "B(AI,BJ)");
  for(h=0; h < nirreps; h++) {
    global_dpd_->buf4_mat_irrep_init(&Bmat, h);
    global_dpd_->buf4_mat_irrep_rd(&Bmat, h);
    for(ai=0; ai < Bmat.params->rowtot[h]; ai++) {
      a = Bmat.params->roworb[h][ai][0];
      i = Bmat.params->roworb[h][ai][1];
      A = fAB.params->rowidx[a];
      I = fIJ.params->rowidx[i];
      Asym = fAB.params->psym[a];
      Isym = fIJ.params->psym[i];
      for(bj=0; bj < Bmat.params->coltot[h]; bj++) {
	b = Bmat.params->colorb[h][bj][0];
	j = Bmat.params->colorb[h][bj][1];
	B = fAB.params->colidx[b];
	J = fIJ.params->colidx[j];
	Bsym = fAB.params->qsym[b];
	Jsym = fIJ.params->qsym[j];
	if((A==B) && (Isym==Jsym)) Bmat.matrix[h][ai][bj] -= fIJ.matrix[Isym][I][J];
	if((I==J) && (Asym==Bsym)) Bmat.matrix[h][ai][bj] += fAB.matrix[Asym][A][B];
	if(ai==bj) Bmat.matrix[h][ai][bj] -= omega;
      }
    }
    global_dpd_->buf4_mat_irrep_wrt(&Bmat, h);
    global_dpd_->buf4_mat_irrep_close(&Bmat, h);
  }
  global_dpd_->buf4_close(&Bmat);
	
  global_dpd_->file2_mat_close(&fab);
  global_dpd_->file2_close(&fab);
  global_dpd_->file2_mat_close(&fAB);
  global_dpd_->file2_close(&fAB);
  global_dpd_->file2_mat_close(&fij);
  global_dpd_->file2_close(&fij);
  global_dpd_->file2_mat_close(&fIJ);
  global_dpd_->file2_close(&fIJ);
	
  psio_close(PSIF_MO_HESS, 1);
}

}} // namespace psi::ccsort
