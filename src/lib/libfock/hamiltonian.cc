#include <libmints/mints.h>
#include <libqt/qt.h>
#include <psi4-dec.h>
#include <boost/tuple/tuple_comparison.hpp>
#include "points.h"
#include "hamiltonian.h"
#include "jk.h"
#include "v.h"

#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace psi;

namespace psi {

Hamiltonian::Hamiltonian(boost::shared_ptr<JK> jk) :
    jk_(jk)
{
    common_init();
}
Hamiltonian::Hamiltonian(boost::shared_ptr<JK> jk, boost::shared_ptr<VBase> v) :
    jk_(jk), v_(v)
{
    common_init();
}
Hamiltonian::~Hamiltonian()
{
}
void Hamiltonian::common_init()
{
    print_ = 1;
    debug_ = 0;
    bench_ = 0;
    exact_diagonal_ = false;
}

RHamiltonian::RHamiltonian(boost::shared_ptr<JK> jk) :
    Hamiltonian(jk)
{
}
RHamiltonian::RHamiltonian(boost::shared_ptr<JK> jk, boost::shared_ptr<VBase> v) :
    Hamiltonian(jk, v)
{
}
RHamiltonian::~RHamiltonian()
{
}
SharedMatrix RHamiltonian::explicit_hamiltonian()
{
    boost::shared_ptr<Vector> diag = diagonal();

    SharedMatrix H(new Matrix("Explicit Hamiltonian", diag->nirrep(), diag->dimpi(), diag->dimpi()));

    boost::shared_ptr<Vector> b(diag->clone());
    boost::shared_ptr<Vector> s(diag->clone());
    std::vector<boost::shared_ptr<Vector> > bb;
    std::vector<boost::shared_ptr<Vector> > ss;
    bb.push_back(b);
    ss.push_back(s);
    for (int h = 0; h < diag->nirrep(); h++) {
        for (int n = 0; n < diag->dimpi()[h]; n++) {
            b->zero();
            s->zero();
            b->set(h,n,1.0);
            product(bb,ss); 
            C_DCOPY(diag->dimpi()[h],s->pointer(h),1,H->pointer(h)[n],1);
        }
    }

    return H;
}

UHamiltonian::UHamiltonian(boost::shared_ptr<JK> jk) :
    Hamiltonian(jk)
{
}
UHamiltonian::UHamiltonian(boost::shared_ptr<JK> jk, boost::shared_ptr<VBase> v) :
    Hamiltonian(jk,v)
{
}
UHamiltonian::~UHamiltonian()
{
}

MatrixRHamiltonian::MatrixRHamiltonian(SharedMatrix M) :
    RHamiltonian(boost::shared_ptr<JK>()), M_(M)
{
}
MatrixRHamiltonian::~MatrixRHamiltonian()
{
}
void MatrixRHamiltonian::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> MatrixRHamiltonian (by Rob Parrish) <== \n\n");
    }
}
boost::shared_ptr<Vector> MatrixRHamiltonian::diagonal()
{
    boost::shared_ptr<Vector> diag(new Vector("Matrix Diagonal", M_->nirrep(), M_->rowspi()));
    for (int h = 0; h < M_->nirrep(); ++h) {
        int n = M_->rowspi()[h];
        if (!n) continue;
        double** Mp = M_->pointer(h);
        double*  Dp = diag->pointer(h);
        for (int i = 0; i < n; ++i) {
            Dp[i] = Mp[i][i];  
        }
    }
    return diag;
}
void MatrixRHamiltonian::product(const std::vector<boost::shared_ptr<Vector> >& x, 
                              std::vector<boost::shared_ptr<Vector> >& b)
{
    for (int N = 0; N < x.size(); ++N) {
        for (int h = 0; h < M_->nirrep(); ++h) {
            int n = M_->rowspi()[h];
            if (!n) continue;
            double** Mp = M_->pointer(h);
            double*  xp = x[N]->pointer(h);    
            double*  bp = b[N]->pointer(h);    
            C_DGEMV('N',n,n,1.0,Mp[0],n,xp,1,0.0,bp,1);
        }
    }
}

MatrixUHamiltonian::MatrixUHamiltonian(std::pair<SharedMatrix, SharedMatrix > M) :
    UHamiltonian(boost::shared_ptr<JK>()), M_(M)
{
}
MatrixUHamiltonian::~MatrixUHamiltonian()
{
}
void MatrixUHamiltonian::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> MatrixUHamiltonian (by Rob Parrish) <== \n\n");
    }
}
std::pair<boost::shared_ptr<Vector>, boost::shared_ptr<Vector> > MatrixUHamiltonian::diagonal()
{
    boost::shared_ptr<Vector> diaga(new Vector("Alpha Matrix Diagonal", M_.first->nirrep(), M_.first->rowspi()));
    boost::shared_ptr<Vector> diagb(new Vector("Beta Matrix Diagonal", M_.first->nirrep(), M_.first->rowspi()));
    for (int h = 0; h < M_.first->nirrep(); ++h) {
        int n = M_.first->rowspi()[h];
        if (!n) continue;
        double** Map = M_.first->pointer(h);
        double*  Dap = diaga->pointer(h);
        double** Mbp = M_.second->pointer(h);
        double*  Dbp = diagb->pointer(h);
        for (int i = 0; i < n; ++i) {
            Dap[i] = Map[i][i];  
            Dbp[i] = Mbp[i][i];  
        }
    }
    return make_pair(diaga,diagb);
}
void MatrixUHamiltonian::product(const std::vector<std::pair<boost::shared_ptr<Vector>,boost::shared_ptr<Vector> > >& x, 
                              std::vector<std::pair<boost::shared_ptr<Vector>,boost::shared_ptr<Vector> > >& b)
{
    for (int N = 0; N < x.size(); ++N) {
        for (int h = 0; h < M_.first->nirrep(); ++h) {
            int n = M_.first->rowspi()[h];
            if (!n) continue;
            double** Map = M_.first->pointer(h);
            double*  xap = x[N].first->pointer(h);    
            double*  bap = b[N].first->pointer(h);    
            C_DGEMV('N',n,n,1.0,Map[0],n,xap,1,0.0,bap,1);
            double** Mbp = M_.second->pointer(h);
            double*  xbp = x[N].second->pointer(h);    
            double*  bbp = b[N].second->pointer(h);    
            C_DGEMV('N',n,n,1.0,Map[0],n,xap,1,0.0,bap,1);
        }
    }
}

CISRHamiltonian::CISRHamiltonian(boost::shared_ptr<JK> jk, 
                                 SharedMatrix Caocc, 
                                 SharedMatrix Cavir, 
                                 boost::shared_ptr<Vector> eps_aocc,
                                 boost::shared_ptr<Vector> eps_avir,
                                 boost::shared_ptr<VBase> v) :
    RHamiltonian(jk,v), Caocc_(Caocc), Cavir_(Cavir), eps_aocc_(eps_aocc), eps_avir_(eps_avir), singlet_(true)
{
}
CISRHamiltonian::~CISRHamiltonian()
{
}
void CISRHamiltonian::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> CISRHamiltonian (by Rob Parrish) <== \n\n");
    }
}
boost::shared_ptr<Vector> CISRHamiltonian::diagonal()
{

    int nirrep = eps_aocc_->nirrep();
    int* nov = new int[nirrep];
    ::memset((void*)nov,'\0',nirrep*sizeof(int));
    for (int symm = 0; symm < nirrep; ++symm) {
        for (int h = 0; h < nirrep; ++h) {
            nov[symm] += eps_aocc_->dimpi()[h] * eps_avir_->dimpi()[symm^h];
        }
    }

    boost::shared_ptr<Vector> diag(new Vector("CIS Diagonal", nirrep, nov));

    for (int symm = 0; symm < nirrep; ++symm) {
        long int offset = 0L;
        for (int h = 0; h < nirrep; ++h) {
            int nocc = eps_aocc_->dimpi()[h];
            int nvir = eps_avir_->dimpi()[symm^h];

            if (!nocc || !nvir) continue;
            
            double* eop = eps_aocc_->pointer(h); 
            double* evp = eps_avir_->pointer(symm^h); 
            double* dp  = diag->pointer(symm); 

            for (int i = 0; i < nocc; ++i) {
                for (int a = 0; a < nvir; ++a) {
                    dp[i * nvir + a + offset] = evp[a] - eop[i];
                }
            }
            offset += nocc * nvir;
        }
    }

    if (exact_diagonal_) {
        try {
            SharedVector iaia = jk_->iaia(Caocc_,Cavir_);
            double factor = (singlet_ ? 1.0 : -1.0);
            for (int symm = 0; symm < nirrep; ++symm) {
                for (int ia = 0; ia < nov[symm]; ia++) {
                    diag->add(symm,ia,iaia->get(symm,ia)); 
                }
            } 
            fprintf(outfile, "    CISR Hamiltonian: (ia|ia) integrals used to form exact diagonal.\n\n");
        } catch (...) {
            fprintf(outfile, "    CISR Hamiltonian: (ia|ia) integrals not available for JK type.\n\n");
        }
    }

    delete[] nov;
    return diag;
}
void CISRHamiltonian::product(const std::vector<boost::shared_ptr<Vector> >& x,
                                    std::vector<boost::shared_ptr<Vector> >& b)
{
    std::vector<SharedMatrix >& C_left = jk_->C_left();
    std::vector<SharedMatrix >& C_right = jk_->C_right();

    C_left.clear();
    C_right.clear();

    int nirrep = (x.size() ? x[0]->nirrep() : 0);

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
            double* xp = x[N]->pointer(symm);
            long int offset = 0L;

            // Is this a delta (low-rank) vector?
            bool delta_vector = true;
            int  delta_h = 0;
            int  delta_i = 0;
            int  delta_a = 0;
            double delta_sum = 0.0;
            for (int h = 0; h < Caocc_->nirrep(); ++h) {
                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                for (int i = 0; i < nocc; i++) {
                    for (int a = 0; a < nvir; a++) {    
                        double x = xp[i * nvir + a + offset];
                        if (x != 0.0 && x != 1.0) {
                            delta_vector = false;
                            break;
                        }
                        delta_sum += x;
                        if (x == 1.0) {
                            delta_i = i;
                            delta_a = a;
                            delta_h = h;
                        }
                    }
                }
                offset += nocc * nvir;
            }
            if (delta_sum != 1.0) {
                delta_vector = false;
            }

            if (delta_vector) {
                
                Dimension rank(Caocc_->nirrep());
                rank[delta_h] = 1;

                std::stringstream ss;
                ss << "C_left, h = " << symm << ", N = " << N;
                SharedMatrix Cl(new Matrix(ss.str(), Caocc_->nirrep(), Caocc_->rowspi(), rank));

                double** Clp = Cl->pointer(delta_h);
                double** Cop = Caocc_->pointer(delta_h);
                C_DCOPY(Caocc_->rowspi()[delta_h],&Cop[0][delta_i],Caocc_->colspi()[delta_h],Clp[0],1);

                std::stringstream ss2;
                ss2 << "C_right, h = " << symm << ", N = " << N;
                SharedMatrix Cr(new Matrix(ss2.str(), Caocc_->nirrep(), Caocc_->rowspi(), rank, symm));

                double** Crp = Cr->pointer(delta_h);
                double** Cvp = Cavir_->pointer(delta_h^symm);
                C_DCOPY(Cavir_->rowspi()[delta_h^symm],&Cvp[0][delta_a],Cavir_->colspi()[delta_h^symm],Crp[0],1);

                C_left.push_back(Cl);
                C_right.push_back(Cr);

            } else {
                std::stringstream ss;
                ss << "C_right, h = " << symm << ", N = " << N;
                SharedMatrix Cr(new Matrix(ss.str(), Caocc_->nirrep(), Caocc_->rowspi(), Caocc_->colspi(), symm));
                
                offset = 0L;
                for (int h = 0; h < Caocc_->nirrep(); ++h) {
    
                    int nocc = Caocc_->colspi()[h];
                    int nvir = Cavir_->colspi()[h^symm];
                    int nso  = Cavir_->rowspi()[h^symm];
    
                    if (!nso || !nocc || !nvir) continue;
    
                    double** Cvp = Cavir_->pointer(h^symm);
                    double** Crp = Cr->pointer(h^symm);
    
                    C_DGEMM('N','T',nso,nocc,nvir,1.0,Cvp[0],nvir,&xp[offset],nvir,0.0,Crp[0],nocc);
    
                    offset += nocc * nvir;
                }
    
                C_left.push_back(Caocc_);
                C_right.push_back(Cr); 
            }
        }
    } 

    jk_->compute();

    const std::vector<SharedMatrix >& J = jk_->J();
    const std::vector<SharedMatrix >& K = jk_->K();

    double* Tp = new double[Caocc_->max_nrow() * Caocc_->max_ncol()];

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
    
            double* bp = b[N]->pointer(symm);
            double* xp = x[N]->pointer(symm);
            long int offset = 0L;

            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nsoocc = Caocc_->rowspi()[h];
                int nocc = Caocc_->colspi()[h];
                int nsovir = Cavir_->rowspi()[h^symm];
                int nvir = Cavir_->colspi()[h^symm];
    
                if (!nsoocc || !nsovir || !nocc || !nvir) continue;
    
                double** Cop = Caocc_->pointer(h);
                double** Cvp = Cavir_->pointer(h^symm);
                double*  eop  = eps_aocc_->pointer(h);
                double*  evp  = eps_avir_->pointer(h^symm);

                double** Jp  = J[symm * x.size() + N]->pointer(h);
                double** Kp  = K[symm * x.size() + N]->pointer(h);
    
                // -(ij|ab)P_jb = C_im K_mn C_na
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,Kp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,0.0,&bp[offset],nvir);
    
                if (singlet_) {
                    // 2(ia|jb)P_jb = C_im J_mn C_na
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,Jp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
                }
            
                for (int i = 0; i < nocc; ++i) {
                    for (int a = 0; a < nvir; ++a) {
                        bp[i * nvir + a + offset] += (evp[a] - eop[i]) * xp[i * nvir + a + offset];        
                    }
                }

                offset += nocc * nvir;
            }        
        }
    }

    delete[] Tp;

    if (debug_ > 3) {
        for (int N = 0; N < x.size(); N++) {
            x[N]->print();
            b[N]->print();
        }
    }
    
}
std::vector<SharedMatrix > CISRHamiltonian::unpack(const boost::shared_ptr<Vector>& eig)
{
    int nirrep = eig->nirrep();
    std::vector<SharedMatrix > t1;
    for (int symm = 0; symm < nirrep; ++symm) {
        SharedMatrix t(new Matrix("T",Caocc_->nirrep(), Caocc_->colspi(), Cavir_->colspi(), symm)); 
        long int offset = 0L;
        for (int h = 0; h < nirrep; ++h) {
            int nocc = Caocc_->colspi()[h];
            int nvir = Cavir_->colspi()[h^symm];
            
            if (!nocc || !nvir) continue;

            ::memcpy((void*)(t->pointer(h)[0]), (void*)(&eig->pointer(symm)[offset]), sizeof(double) * nocc * nvir);    
        
            offset += nocc * nvir;
        }    

        t1.push_back(t);
    }

    return t1;
}
TDHFRHamiltonian::TDHFRHamiltonian(boost::shared_ptr<JK> jk, 
                                 SharedMatrix Caocc, 
                                 SharedMatrix Cavir, 
                                 boost::shared_ptr<Vector> eps_aocc,
                                 boost::shared_ptr<Vector> eps_avir,
                                 boost::shared_ptr<VBase> v) : 
    RHamiltonian(jk,v), Caocc_(Caocc), Cavir_(Cavir), eps_aocc_(eps_aocc), eps_avir_(eps_avir), singlet_(true) 
{
}
TDHFRHamiltonian::~TDHFRHamiltonian()
{
}
void TDHFRHamiltonian::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> TDHFRHamiltonian (by Rob Parrish) <== \n\n");
    }
}
boost::shared_ptr<Vector> TDHFRHamiltonian::diagonal()
{
    int nirrep = eps_aocc_->nirrep();
    int* nov = new int[nirrep];
    ::memset((void*)nov,'\0',nirrep*sizeof(int));
    for (int symm = 0; symm < nirrep; ++symm) {
        for (int h = 0; h < nirrep; ++h) {
            nov[symm] += 2 * eps_aocc_->dimpi()[h] * eps_avir_->dimpi()[symm^h];
        }
    }

    boost::shared_ptr<Vector> diag(new Vector("TDHF Diagonal", nirrep, nov));

    for (int symm = 0; symm < nirrep; ++symm) {
        long int offset = 0L;
        for (int h = 0; h < nirrep; ++h) {
            int nov = diag->dimpi()[h] / 2;
            int nocc = eps_aocc_->dimpi()[h];
            int nvir = eps_avir_->dimpi()[symm^h];

            if (!nocc || !nvir) continue;
            
            double* eop = eps_aocc_->pointer(h); 
            double* evp = eps_avir_->pointer(symm^h); 
            double* dp  = diag->pointer(symm); 

            for (int i = 0; i < nocc; ++i) {
                for (int a = 0; a < nvir; ++a) {
                    dp[i * nvir + a + offset] = (evp[a] - eop[i]);
                    dp[i * nvir + a + offset + nov] = -(evp[a] - eop[i]);
                }
            }
            offset += nocc * nvir;
        }
    }

    delete[] nov;
    return diag;
}
void TDHFRHamiltonian::product(const std::vector<boost::shared_ptr<Vector> >& x,
                                     std::vector<boost::shared_ptr<Vector> >& b)
{
    std::vector<SharedMatrix >& C_left = jk_->C_left();
    std::vector<SharedMatrix >& C_right = jk_->C_right();

    C_left.clear();
    C_right.clear();

    int nirrep = (x.size() ? x[0]->nirrep() : 0);

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {

            C_left.push_back(Caocc_);
            C_left.push_back(Caocc_);

            int nov = x[N]->dimpi()[symm] / 2;

            double* xp = x[N]->pointer(symm);

            std::stringstream ss;
            ss << "C_right_X, h = " << symm << ", N = " << N;
            SharedMatrix CXr(new Matrix(ss.str(), Caocc_->nirrep(), Caocc_->rowspi(), Caocc_->colspi(), symm));
            
            long int offset = 0L;
            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                int nso  = Cavir_->rowspi()[h^symm];

                if (!nso || !nocc || !nvir) continue;

                double** Cvp = Cavir_->pointer(h^symm);
                double** Crp = CXr->pointer(h^symm);

                C_DGEMM('N','T',nso,nocc,nvir,1.0,Cvp[0],nvir,&xp[offset],nvir,0.0,Crp[0],nocc);

                offset += nocc * nvir;
            }

            C_right.push_back(CXr); 

            std::stringstream ss2;
            ss2 << "C_right_Y, h = " << symm << ", N = " << N;
            SharedMatrix CYr(new Matrix(ss.str(), Caocc_->nirrep(), Caocc_->rowspi(), Caocc_->colspi(), symm));
            
            offset = 0L;
            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                int nso  = Cavir_->rowspi()[h^symm];

                if (!nso || !nocc || !nvir) continue;

                double** Cvp = Cavir_->pointer(h^symm);
                double** Crp = CYr->pointer(h^symm);

                C_DGEMM('N','T',nso,nocc,nvir,1.0,Cvp[0],nvir,&xp[offset + nov],nvir,0.0,Crp[0],nocc);

                offset += nocc * nvir;
            }

            C_right.push_back(CYr); 
        }
    }
    
    jk_->compute();

    const std::vector<SharedMatrix >& J = jk_->J();
    const std::vector<SharedMatrix >& K = jk_->K();

    double* Tp = new double[Caocc_->max_nrow() * Caocc_->max_ncol()];

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
    
            int nov = x[N]->dimpi()[symm] / 2;

            double* bp = b[N]->pointer(symm);
            double* xp = x[N]->pointer(symm);
            long int offset = 0L;

            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nsoocc = Caocc_->rowspi()[h];
                int nocc = Caocc_->colspi()[h];
                int nsovir = Cavir_->rowspi()[h^symm];
                int nvir = Cavir_->colspi()[h^symm];
    
                if (!nsoocc || !nsovir || !nocc || !nvir) continue;
    
                double** Cop = Caocc_->pointer(h);
                double** Cvp = Cavir_->pointer(h^symm);
                double*  eop  = eps_aocc_->pointer(h);
                double*  evp  = eps_avir_->pointer(h^symm);

                double** JXp   = J[2 * symm * x.size() + 2 * N]->pointer(h);
                double** KXp   = K[2 * symm * x.size() + 2 * N]->pointer(h);
                double** KX2p  = K[2 * symm * x.size() + 2 * N]->pointer(h^symm);
                double** JYp   = J[2 * symm * x.size() + 2 * N + 1]->pointer(h);
                double** KYp   = K[2 * symm * x.size() + 2 * N + 1]->pointer(h);
                double** KY2p  = K[2 * symm * x.size() + 2 * N + 1]->pointer(h^symm);
    
                // A terms

                // -(ij|ab)P_jb = C_im K_mn C_na
                // AX -> SX
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,KXp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,0.0,&bp[offset],nvir);
                // -AY -> SY
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,KYp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,1.0,Tp,nsovir,Cvp[0],nvir,0.0,&bp[offset + nov],nvir);
    
                if (singlet_) {
                    // 2(ia|jb)P_jb = C_im J_mn C_na
                    // AX -> SX
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,JXp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
                    // -AY -> SY
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,JYp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,-2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset + nov],nvir);
                }
            
                // B terms

                // -(ib|ja)P_jb = C_in K_nm C_ma
                // BY -> SX
                C_DGEMM('T','T',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,KY2p[0],nsoocc,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
                // -BX -> SY
                C_DGEMM('T','T',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,KX2p[0],nsoocc,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,1.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset + nov],nvir);

                if (singlet_) {
                    // 2(ia|jb)P_jb = C_im J_mn C_na
                    // BY -> SX
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,JYp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
                    // -BX -> SY
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,JXp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,-2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset + nov],nvir);
                }

                // Diagonal terms
            
                for (int i = 0; i < nocc; ++i) {
                    for (int a = 0; a < nvir; ++a) {
                        bp[i * nvir + a + offset] += (evp[a] - eop[i]) * xp[i * nvir + a + offset];        
                        bp[i * nvir + a + offset + nov] -= (evp[a] - eop[i]) * xp[i * nvir + a + offset + nov];        
                    }
                }

                offset += nocc * nvir;
            }        
        }
    }

    delete[] Tp;

    if (debug_ > 3) {
        for (int N = 0; N < x.size(); N++) {
            x[N]->print();
            b[N]->print();
        }
    }
}
CPHFRHamiltonian::CPHFRHamiltonian(boost::shared_ptr<JK> jk, 
                                   SharedMatrix Caocc, 
                                   SharedMatrix Cavir, 
                                   boost::shared_ptr<Vector> eps_aocc,
                                   boost::shared_ptr<Vector> eps_avir,
                                   boost::shared_ptr<VBase> v) : 
    RHamiltonian(jk,v), Caocc_(Caocc), Cavir_(Cavir), eps_aocc_(eps_aocc), eps_avir_(eps_avir)
{
}
CPHFRHamiltonian::~CPHFRHamiltonian()
{
}
void CPHFRHamiltonian::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> CPHFRHamiltonian (by Rob Parrish) <== \n\n");
    }
}
boost::shared_ptr<Vector> CPHFRHamiltonian::diagonal()
{
    int nirrep = eps_aocc_->nirrep();
    int* nov = new int[nirrep];
    ::memset((void*)nov,'\0',nirrep*sizeof(int));
    for (int symm = 0; symm < nirrep; ++symm) {
        for (int h = 0; h < nirrep; ++h) {
            nov[symm] += eps_aocc_->dimpi()[h] * eps_avir_->dimpi()[symm^h];
        }
    }

    boost::shared_ptr<Vector> diag(new Vector("CPHF Diagonal", nirrep, nov));

    for (int symm = 0; symm < nirrep; ++symm) {
        long int offset = 0L;
        for (int h = 0; h < nirrep; ++h) {
            int nocc = eps_aocc_->dimpi()[h];
            int nvir = eps_avir_->dimpi()[symm^h];

            if (!nocc || !nvir) continue;
            
            double* eop = eps_aocc_->pointer(h); 
            double* evp = eps_avir_->pointer(symm^h); 
            double* dp  = diag->pointer(symm); 

            for (int i = 0; i < nocc; ++i) {
                for (int a = 0; a < nvir; ++a) {
                    dp[i * nvir + a + offset] = evp[a] - eop[i];
                }
            }
            offset += nocc * nvir;
        }
    }

    delete[] nov;
    return diag;
}
std::map<std::string, SharedVector> CPHFRHamiltonian::pack(const std::map<std::string, SharedMatrix>& x)
{
    int nirrep = eps_aocc_->nirrep();
    int* nov = new int[nirrep];
    ::memset((void*)nov,'\0',nirrep*sizeof(int));
    for (int symm = 0; symm < nirrep; ++symm) {
        for (int h = 0; h < nirrep; ++h) {
            nov[symm] += eps_aocc_->dimpi()[h] * eps_avir_->dimpi()[symm^h];
        }
    }

    std::map<std::string, SharedVector> X;
    for (std::map<std::string, SharedMatrix>::const_iterator it = x.begin(); 
        it != x.end(); ++it) {

        boost::shared_ptr<Vector> v(new Vector("X", nirrep, nov));
        SharedMatrix x2 = (*it).second;
        int symm = x2->symmetry();
        int offset = 0;
        for (int h = 0; h < nirrep; h++) {
            int nocc = eps_aocc_->dimpi()[h];
            int nvir = eps_avir_->dimpi()[h^symm];

            if (!nocc || !nvir) continue;

            ::memcpy((void*) &v->pointer(symm)[offset], (void*) x2->pointer(h)[0], sizeof(double)*nocc*nvir);

            offset += nocc * nvir;
        } 
        X[(*it).first] = v;
    }
    return X;
}
void CPHFRHamiltonian::product(const std::vector<boost::shared_ptr<Vector> >& x,
                                     std::vector<boost::shared_ptr<Vector> >& b)
{
    std::vector<SharedMatrix >& C_left = jk_->C_left();
    std::vector<SharedMatrix >& C_right = jk_->C_right();

    C_left.clear();
    C_right.clear();

    int nirrep = (x.size() ? x[0]->nirrep() : 0);

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
            C_left.push_back(Caocc_);
            double* xp = x[N]->pointer(symm);

            std::stringstream ss;
            ss << "C_right, h = " << symm << ", N = " << N;
            SharedMatrix Cr(new Matrix(ss.str(), Caocc_->nirrep(), Caocc_->rowspi(), Caocc_->colspi(), symm));
            
            long int offset = 0L;
            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                int nso  = Cavir_->rowspi()[h^symm];

                if (!nso || !nocc || !nvir) continue;

                double** Cvp = Cavir_->pointer(h^symm);
                double** Crp = Cr->pointer(h^symm);

                C_DGEMM('N','T',nso,nocc,nvir,1.0,Cvp[0],nvir,&xp[offset],nvir,0.0,Crp[0],nocc);

                offset += nocc * nvir;
            }

            C_right.push_back(Cr); 
        }
    }
    
    jk_->compute();

    const std::vector<SharedMatrix >& J = jk_->J();
    const std::vector<SharedMatrix >& K = jk_->K();

    double* Tp = new double[Caocc_->max_nrow() * Caocc_->max_ncol()];

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
    
            double* bp = b[N]->pointer(symm);
            double* xp = x[N]->pointer(symm);
            long int offset = 0L;

            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nsoocc = Caocc_->rowspi()[h];
                int nocc = Caocc_->colspi()[h];
                int nsovir = Cavir_->rowspi()[h^symm];
                int nvir = Cavir_->colspi()[h^symm];
    
                if (!nsoocc || !nsovir || !nocc || !nvir) continue;
    
                double** Cop = Caocc_->pointer(h);
                double** Cvp = Cavir_->pointer(h^symm);
                double*  eop  = eps_aocc_->pointer(h);
                double*  evp  = eps_avir_->pointer(h^symm);
                double** Jp  = J[N]->pointer(h);
                double** Kp  = K[N]->pointer(h);
                double** K2p = K[N]->pointer(h^symm);
    
                // 4(ia|jb)P_jb = C_im J_mn C_na
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,Jp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,4.0,Tp,nsovir,Cvp[0],nvir,0.0,&bp[offset],nvir);
    
                // -(ib|ja)P_jb = C_in K_nm C_ma
                C_DGEMM('T','T',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,K2p[0],nsoocc,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
    
                // -(ij|ab)P_jb = C_im K_mn C_ra
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,Kp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
    
                for (int i = 0; i < nocc; ++i) {
                    for (int a = 0; a < nvir; ++a) {
                        bp[i * nvir + a + offset] += (evp[a] - eop[i]) * xp[i * nvir + a + offset];        
                    }
                }

                offset += nocc * nvir;
            }        
        }
    }

    delete[] Tp;

    if (debug_ > 3) {
        for (int N = 0; N < x.size(); N++) {
            x[N]->print();
            b[N]->print();
        }
    }
}
std::vector<SharedMatrix > CPHFRHamiltonian::unpack(const std::vector<boost::shared_ptr<Vector> >& x)
{
    std::vector<SharedMatrix > t1;
    int nirrep = x[0]->nirrep();
    for (int i = 0; i < x.size(); i++) {
        for (int symm = 0; symm < nirrep; ++symm) {
            SharedMatrix t(new Matrix("X",Caocc_->nirrep(), Caocc_->colspi(), Cavir_->colspi(), symm)); 
            long int offset = 0L;
            for (int h = 0; h < nirrep; ++h) {
                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                
                if (!nocc || !nvir) continue;

                ::memcpy((void*)(t->pointer(h)[0]), (void*)(&x[i]->pointer(symm)[offset]), sizeof(double) * nocc * nvir);    
            
                offset += nocc * nvir;
            }    

            t1.push_back(t);
        }
    }

    return t1;
}

TDARHamiltonian::TDARHamiltonian(boost::shared_ptr<JK> jk, 
                    boost::shared_ptr<VBase> v,
                    SharedMatrix Cocc,
                    SharedMatrix Caocc, 
                    SharedMatrix Cavir, 
                    boost::shared_ptr<Vector> eps_aocc,
                    boost::shared_ptr<Vector> eps_avir) : 
    CISRHamiltonian(jk,Caocc,Cavir,eps_aocc,eps_avir,v), Cocc_(Cocc)
{
    std::vector<SharedMatrix>& C = v->C();
    C.clear();
    C.push_back(Cocc);

    std::vector<SharedMatrix>& Cao = v->Caocc();
    Cao.clear();
    Cao.push_back(Caocc);

    std::vector<SharedMatrix>& Cav = v->Cavir();
    Cav.clear();
    Cav.push_back(Cavir);
}
TDARHamiltonian::~TDARHamiltonian()
{
}
void TDARHamiltonian::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> TDARHamiltonian (by Rob Parrish) <== \n\n");
    }
}
void TDARHamiltonian::product(const std::vector<boost::shared_ptr<Vector> >& x,
                                     std::vector<boost::shared_ptr<Vector> >& b)
{
    std::vector<SharedMatrix >& C_left = jk_->C_left();
    std::vector<SharedMatrix >& C_right = jk_->C_right();
    std::vector<SharedMatrix >& P = v_->P();

    C_left.clear();
    C_right.clear();
    P.clear();

    int nirrep = (x.size() ? x[0]->nirrep() : 0);

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
            C_left.push_back(Caocc_);
            double* xp = x[N]->pointer(symm);

            std::stringstream ss;
            ss << "C_right, h = " << symm << ", N = " << N;
            SharedMatrix Cr(new Matrix(ss.str(), Caocc_->nirrep(), Caocc_->rowspi(), Caocc_->colspi(), symm));
            
            long int offset = 0L;
            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                int nso  = Cavir_->rowspi()[h^symm];

                if (!nso || !nocc || !nvir) continue;

                double** Cvp = Cavir_->pointer(h^symm);
                double** Crp = Cr->pointer(h^symm);

                C_DGEMM('N','T',nso,nocc,nvir,1.0,Cvp[0],nvir,&xp[offset],nvir,0.0,Crp[0],nocc);

                offset += nocc * nvir;
            }

            C_right.push_back(Cr); 

            std::stringstream ss2;
            ss2 << "P, h = " << symm << ", N = " << N;
            SharedMatrix P2(new Matrix(ss2.str(), Caocc_->colspi(), Cavir_->colspi(), symm));

            offset = 0L;
            for (int h = 0; h < Caocc_->nirrep(); ++h) {
                double* xp = x[N]->pointer(symm);
                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                if (!nocc || ! nvir) continue;
                double** P2p = P2->pointer(h);
                ::memcpy((void*) P2p[0], (void*) &xp[offset], sizeof(double) * nocc * nvir);
                offset += nocc * nvir;
            }

            P.push_back(P2);
        }
    }

    jk_->compute();
    v_->compute();

    const std::vector<SharedMatrix >& J = jk_->J();
    const std::vector<SharedMatrix >& K = jk_->K();
    const std::vector<SharedMatrix >& V = v_->V();

    double* Tp = new double[Caocc_->max_nrow() * Caocc_->max_ncol()];

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
    
            double* bp = b[N]->pointer(symm);
            double* xp = x[N]->pointer(symm);
            long int offset = 0L;

            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nsoocc = Caocc_->rowspi()[h];
                int nocc = Caocc_->colspi()[h];
                int nsovir = Cavir_->rowspi()[h^symm];
                int nvir = Cavir_->colspi()[h^symm];
    
                if (!nsoocc || !nsovir || !nocc || !nvir) continue;
    
                double** Cop = Caocc_->pointer(h);
                double** Cvp = Cavir_->pointer(h^symm);
                double*  eop  = eps_aocc_->pointer(h);
                double*  evp  = eps_avir_->pointer(h^symm);

                double** Jp  = J[symm * x.size() + N]->pointer(h);
                double** Kp  = K[symm * x.size() + N]->pointer(h);
                double** Vp  = V[symm * x.size() + N]->pointer(h);
    
                // TODO: hybrid/RC terms
                // -(ij|ab)P_jb = C_im K_mn C_na
                //C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,Kp[0],nsovir,0.0,Tp,nsovir);
                //C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,0.0,&bp[offset],nvir);
    
                if (singlet_) {
                    // 2(ia|jb)P_jb = C_im J_mn C_na
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,Jp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
                }
            
                // V matrix
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,Vp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,1.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);

                for (int i = 0; i < nocc; ++i) {
                    for (int a = 0; a < nvir; ++a) {
                        bp[i * nvir + a + offset] += (evp[a] - eop[i]) * xp[i * nvir + a + offset];        
                    }
                }

                offset += nocc * nvir;
            }        
        }
    }

    delete[] Tp;

    if (debug_ > 3) {
        for (int N = 0; N < x.size(); N++) {
            x[N]->print();
            b[N]->print();
        }
    }
}

TDDFTRHamiltonian::TDDFTRHamiltonian(boost::shared_ptr<JK> jk, 
                    boost::shared_ptr<VBase> v,
                    SharedMatrix Cocc,
                    SharedMatrix Caocc, 
                    SharedMatrix Cavir, 
                    boost::shared_ptr<Vector> eps_aocc,
                    boost::shared_ptr<Vector> eps_avir) : 
    TDHFRHamiltonian(jk,Caocc,Cavir,eps_aocc,eps_avir,v), Cocc_(Cocc)
{
    std::vector<SharedMatrix>& C = v->C();
    C.clear();
    C.push_back(Cocc);

    std::vector<SharedMatrix>& Cao = v->Caocc();
    Cao.clear();
    Cao.push_back(Caocc);

    std::vector<SharedMatrix>& Cav = v->Cavir();
    Cav.clear();
    Cav.push_back(Cavir);
}
TDDFTRHamiltonian::~TDDFTRHamiltonian()
{
}
void TDDFTRHamiltonian::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> TDDFTHamiltonian (by Rob Parrish) <== \n\n");
    }
}
void TDDFTRHamiltonian::product(const std::vector<boost::shared_ptr<Vector> >& x,
                                     std::vector<boost::shared_ptr<Vector> >& b)
{
    // TODO V

    std::vector<SharedMatrix >& C_left = jk_->C_left();
    std::vector<SharedMatrix >& C_right = jk_->C_right();

    C_left.clear();
    C_right.clear();

    int nirrep = (x.size() ? x[0]->nirrep() : 0);

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {

            C_left.push_back(Caocc_);
            C_left.push_back(Caocc_);

            int nov = x[N]->dimpi()[symm] / 2;

            double* xp = x[N]->pointer(symm);

            std::stringstream ss;
            ss << "C_right_X, h = " << symm << ", N = " << N;
            SharedMatrix CXr(new Matrix(ss.str(), Caocc_->nirrep(), Caocc_->rowspi(), Caocc_->colspi(), symm));
            
            long int offset = 0L;
            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                int nso  = Cavir_->rowspi()[h^symm];

                if (!nso || !nocc || !nvir) continue;

                double** Cvp = Cavir_->pointer(h^symm);
                double** Crp = CXr->pointer(h^symm);

                C_DGEMM('N','T',nso,nocc,nvir,1.0,Cvp[0],nvir,&xp[offset],nvir,0.0,Crp[0],nocc);

                offset += nocc * nvir;
            }

            C_right.push_back(CXr); 

            std::stringstream ss2;
            ss2 << "C_right_Y, h = " << symm << ", N = " << N;
            SharedMatrix CYr(new Matrix(ss.str(), Caocc_->nirrep(), Caocc_->rowspi(), Caocc_->colspi(), symm));
            
            offset = 0L;
            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                int nso  = Cavir_->rowspi()[h^symm];

                if (!nso || !nocc || !nvir) continue;

                double** Cvp = Cavir_->pointer(h^symm);
                double** Crp = CYr->pointer(h^symm);

                C_DGEMM('N','T',nso,nocc,nvir,1.0,Cvp[0],nvir,&xp[offset + nov],nvir,0.0,Crp[0],nocc);

                offset += nocc * nvir;
            }

            C_right.push_back(CYr); 
        }
    }
    
    jk_->compute();

    const std::vector<SharedMatrix >& J = jk_->J();
    const std::vector<SharedMatrix >& K = jk_->K();

    double* Tp = new double[Caocc_->max_nrow() * Caocc_->max_ncol()];

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
    
            int nov = x[N]->dimpi()[symm] / 2;

            double* bp = b[N]->pointer(symm);
            double* xp = x[N]->pointer(symm);
            long int offset = 0L;

            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nsoocc = Caocc_->rowspi()[h];
                int nocc = Caocc_->colspi()[h];
                int nsovir = Cavir_->rowspi()[h^symm];
                int nvir = Cavir_->colspi()[h^symm];
    
                if (!nsoocc || !nsovir || !nocc || !nvir) continue;
    
                double** Cop = Caocc_->pointer(h);
                double** Cvp = Cavir_->pointer(h^symm);
                double*  eop  = eps_aocc_->pointer(h);
                double*  evp  = eps_avir_->pointer(h^symm);

                double** JXp   = J[2 * symm * x.size() + 2 * N]->pointer(h);
                double** KXp   = K[2 * symm * x.size() + 2 * N]->pointer(h);
                double** KX2p  = K[2 * symm * x.size() + 2 * N]->pointer(h^symm);
                double** JYp   = J[2 * symm * x.size() + 2 * N + 1]->pointer(h);
                double** KYp   = K[2 * symm * x.size() + 2 * N + 1]->pointer(h);
                double** KY2p  = K[2 * symm * x.size() + 2 * N + 1]->pointer(h^symm);
    
                // A terms

                // -(ij|ab)P_jb = C_im K_mn C_na
                // AX -> SX
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,KXp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,0.0,&bp[offset],nvir);
                // -AY -> SY
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,KYp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,1.0,Tp,nsovir,Cvp[0],nvir,0.0,&bp[offset + nov],nvir);
    
                if (singlet_) {
                    // 2(ia|jb)P_jb = C_im J_mn C_na
                    // AX -> SX
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,JXp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
                    // -AY -> SY
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,JYp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,-2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset + nov],nvir);
                }
            
                // B terms

                // -(ib|ja)P_jb = C_in K_nm C_ma
                // BY -> SX
                C_DGEMM('T','T',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,KY2p[0],nsoocc,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
                // -BX -> SY
                C_DGEMM('T','T',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,KX2p[0],nsoocc,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,1.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset + nov],nvir);

                if (singlet_) {
                    // 2(ia|jb)P_jb = C_im J_mn C_na
                    // BY -> SX
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,JYp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
                    // -BX -> SY
                    C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,JXp[0],nsovir,0.0,Tp,nsovir);
                    C_DGEMM('N','N',nocc,nvir,nsovir,-2.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset + nov],nvir);
                }

                // Diagonal terms
            
                for (int i = 0; i < nocc; ++i) {
                    for (int a = 0; a < nvir; ++a) {
                        bp[i * nvir + a + offset] += (evp[a] - eop[i]) * xp[i * nvir + a + offset];        
                        bp[i * nvir + a + offset + nov] -= (evp[a] - eop[i]) * xp[i * nvir + a + offset + nov];        
                    }
                }

                offset += nocc * nvir;
            }        
        }
    }

    delete[] Tp;

    if (debug_ > 3) {
        for (int N = 0; N < x.size(); N++) {
            x[N]->print();
            b[N]->print();
        }
    }
}

CPKSRHamiltonian::CPKSRHamiltonian(boost::shared_ptr<JK> jk, 
                    boost::shared_ptr<VBase> v,
                    SharedMatrix Cocc,
                    SharedMatrix Caocc, 
                    SharedMatrix Cavir, 
                    boost::shared_ptr<Vector> eps_aocc,
                    boost::shared_ptr<Vector> eps_avir) : 
    CPHFRHamiltonian(jk,Caocc,Cavir,eps_aocc,eps_avir,v), Cocc_(Cocc)
{
    std::vector<SharedMatrix>& C = v->C();
    C.clear();
    C.push_back(Cocc);

    std::vector<SharedMatrix>& Cao = v->Caocc();
    Cao.clear();
    Cao.push_back(Caocc);

    std::vector<SharedMatrix>& Cav = v->Cavir();
    Cav.clear();
    Cav.push_back(Cavir);
}
CPKSRHamiltonian::~CPKSRHamiltonian()
{
}
void CPKSRHamiltonian::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> CPKSRHamiltonian (by Rob Parrish) <== \n\n");
    }
}
void CPKSRHamiltonian::product(const std::vector<boost::shared_ptr<Vector> >& x,
                                     std::vector<boost::shared_ptr<Vector> >& b)
{
    // TODO V

    std::vector<SharedMatrix >& C_left = jk_->C_left();
    std::vector<SharedMatrix >& C_right = jk_->C_right();

    C_left.clear();
    C_right.clear();

    int nirrep = (x.size() ? x[0]->nirrep() : 0);

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
            C_left.push_back(Caocc_);
            double* xp = x[N]->pointer(symm);

            std::stringstream ss;
            ss << "C_right, h = " << symm << ", N = " << N;
            SharedMatrix Cr(new Matrix(ss.str(), Caocc_->nirrep(), Caocc_->rowspi(), Caocc_->colspi(), symm));
            
            long int offset = 0L;
            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nocc = Caocc_->colspi()[h];
                int nvir = Cavir_->colspi()[h^symm];
                int nso  = Cavir_->rowspi()[h^symm];

                if (!nso || !nocc || !nvir) continue;

                double** Cvp = Cavir_->pointer(h^symm);
                double** Crp = Cr->pointer(h^symm);

                C_DGEMM('N','T',nso,nocc,nvir,1.0,Cvp[0],nvir,&xp[offset],nvir,0.0,Crp[0],nocc);

                offset += nocc * nvir;
            }

            C_right.push_back(Cr); 
        }
    }
    
    jk_->compute();

    const std::vector<SharedMatrix >& J = jk_->J();
    const std::vector<SharedMatrix >& K = jk_->K();

    double* Tp = new double[Caocc_->max_nrow() * Caocc_->max_ncol()];

    for (int symm = 0; symm < nirrep; ++symm) {
        for (int N = 0; N < x.size(); ++N) {
    
            double* bp = b[N]->pointer(symm);
            double* xp = x[N]->pointer(symm);
            long int offset = 0L;

            for (int h = 0; h < Caocc_->nirrep(); ++h) {

                int nsoocc = Caocc_->rowspi()[h];
                int nocc = Caocc_->colspi()[h];
                int nsovir = Cavir_->rowspi()[h^symm];
                int nvir = Cavir_->colspi()[h^symm];
    
                if (!nsoocc || !nsovir || !nocc || !nvir) continue;
    
                double** Cop = Caocc_->pointer(h);
                double** Cvp = Cavir_->pointer(h^symm);
                double*  eop  = eps_aocc_->pointer(h);
                double*  evp  = eps_avir_->pointer(h^symm);
                double** Jp  = J[N]->pointer(h);
                double** Kp  = K[N]->pointer(h);
                double** K2p = K[N]->pointer(h^symm);
    
                // 4(ia|jb)P_jb = C_im J_mn C_na
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,Jp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,4.0,Tp,nsovir,Cvp[0],nvir,0.0,&bp[offset],nvir);
    
                // -(ib|ja)P_jb = C_in K_nm C_ma
                C_DGEMM('T','T',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,K2p[0],nsoocc,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
    
                // -(ij|ab)P_jb = C_im K_mn C_ra
                C_DGEMM('T','N',nocc,nsovir,nsoocc,1.0,Cop[0],nocc,Kp[0],nsovir,0.0,Tp,nsovir);
                C_DGEMM('N','N',nocc,nvir,nsovir,-1.0,Tp,nsovir,Cvp[0],nvir,1.0,&bp[offset],nvir);
    
                for (int i = 0; i < nocc; ++i) {
                    for (int a = 0; a < nvir; ++a) {
                        bp[i * nvir + a + offset] += (evp[a] - eop[i]) * xp[i * nvir + a + offset];        
                    }
                }

                offset += nocc * nvir;
            }        
        }
    }

    delete[] Tp;

    if (debug_ > 3) {
        for (int N = 0; N < x.size(); N++) {
            x[N]->print();
            b[N]->print();
        }
    }
}

 
}
