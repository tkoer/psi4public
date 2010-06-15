#include "diismanager.h"
#include <cstdarg>
#include <libdpd/dpd.h>
#include <libmints/matrix.h>
#include <libmints/vector.h>
#include <libciomr/libciomr.h>
#include <libqt/qt.h>
#include <libpsio/psio.hpp>
#include <psifiles.h>

namespace psi{

DIISManager::DIISManager(int maxSubspaceSize,
                         std::string label,
                         RemovalPolicy removalPolicy,
                         StoragePolicy storagePolicy):
            _maxSubspaceSize(maxSubspaceSize),
            _removalPolicy(removalPolicy),
            _storagePolicy(storagePolicy),
            _errorVectorSize(0),
            _vectorSize(0),
            _entryCount(0),
            _label(label)
{
}

/**
 * Determines the size of the error vector from a list of input quantities
 */
void
DIISManager::set_vector_size(int numQuantities, ...)
{
    if(_vectorSize)
        throw SanityCheckError("The size of the DIIS vector has already been set",
                               __FILE__, __LINE__);
    if(_errorVectorSize == 0)
        throw SanityCheckError("The error vector size must be set before the vector size",
                               __FILE__, __LINE__);
    _numVectorComponents = numQuantities;
    dpdfile2 *file2;
    dpdbuf4 *buf4;
    Vector *vector;
    Matrix *matrix;
    va_list args;
    va_start(args, numQuantities);
    for(int i=0; i < numQuantities; ++i) {
        DIISEntry::InputType type = static_cast<DIISEntry::InputType>(va_arg(args, int));
        _componentTypes.push_back(type);
        size_t size = 0;
        switch(type){
            case DIISEntry::Pointer:
                size += va_arg(args, int);
                break;
            case DIISEntry::DPDBuf4:
                buf4 = va_arg(args, dpdbuf4*);
                for(int h = 0; h < buf4->params->nirreps; ++h){
                    size += buf4->params->rowtot[h] * buf4->params->coltot[h];
                }
                break;
            case DIISEntry::DPDFile2:
                file2 = va_arg(args, dpdfile2*);
                for(int h = 0; h < file2->params->nirreps; ++h){
                    size += file2->params->rowtot[h] * file2->params->coltot[h];
                }
                break;
            case DIISEntry::Matrix:
                matrix = va_arg(args, Matrix*);
                for(int h = 0; h < matrix->nirreps(); ++h){
                    size += matrix->rowspi()[h] * matrix->colspi()[h];
                }
                break;
            case DIISEntry::Vector:
                vector = va_arg(args, Vector*);
                for(int h = 0; h < vector->nirreps(); ++h){
                    size += vector->dimpi()[h];
                }
                break;
            default:
                throw SanityCheckError("Unknown input type", __FILE__, __LINE__);
        }
        _componentSizes.push_back(size);
        _vectorSize += size;
    }
    va_end(args);
}


/**
 * Determines the size of the error vector from a list of input quantities.  This function should
 * not be called until set_vector_size() has been called.
 */
void
DIISManager::set_error_vector_size(int numQuantities, ...)
{
    if(_errorVectorSize)
        throw SanityCheckError("The size of the DIIS error vector has already been set",
                               __FILE__, __LINE__);
    _numErrorVectorComponents = numQuantities;
    dpdfile2 *file2;
    dpdbuf4 *buf4;
    Vector *vector;
    Matrix *matrix;
    va_list args;
    va_start(args, numQuantities);
    for(int i=0; i < numQuantities; ++i) {
        DIISEntry::InputType type = static_cast<DIISEntry::InputType>(va_arg(args, int));
        _componentTypes.push_back(type);
        size_t size = 0;
        switch(type){
            case DIISEntry::Pointer:
                size += va_arg(args, int);
                break;
            case DIISEntry::DPDBuf4:
                buf4 = va_arg(args, dpdbuf4*);
                for(int h = 0; h < buf4->params->nirreps; ++h){
                    size += buf4->params->rowtot[h] * buf4->params->coltot[h];
                }
                break;
            case DIISEntry::DPDFile2:
                file2 = va_arg(args, dpdfile2*);
                for(int h = 0; h < file2->params->nirreps; ++h){
                    size += file2->params->rowtot[h] * file2->params->coltot[h];
                }
                break;
            case DIISEntry::Matrix:
                matrix = va_arg(args, Matrix*);
                for(int h = 0; h < matrix->nirreps(); ++h){
                    size += matrix->rowspi()[h] * matrix->colspi()[h];
                }
                break;
            case DIISEntry::Vector:
                vector = va_arg(args, Vector*);
                for(int h = 0; h < vector->nirreps(); ++h){
                    size += vector->dimpi()[h];
                }
                break;
            default:
                throw SanityCheckError("Unknown input type", __FILE__, __LINE__);
        }
        _componentSizes.push_back(size);
        _errorVectorSize += size;
    }
    va_end(args);
}


/**
 * Adds a new vector and error vector to the DIIS subspace.
 * @param numQuantities - the number of components used to construct the
 *        vector and error vector.  Must match the sum of the numbers passed
 *        to the set_vector_size() and set_error_vector_size() functions.
 * The remaining parameters are then the components of the vector and then the error
 * vector, in that order, and in the same order they were passed to the
 * set_vector_size() and set_error_vector_size() functions.  N.B. Unlike the set_size
 * functions, the types of each component should not be specified here.  If the component
 * is an array, the pointer to the start of that array should be passed, in contranst to the
 * set_size functions, which takes only the size of that array.
 * @return Whether the subspace was updated
 */
bool
DIISManager::add_entry(int numQuantities, ...)
{
    if(!_maxSubspaceSize) return false;
    if(_componentSizes.size() != numQuantities)
        throw SanityCheckError("The number of parameters passed to the set_size routines"
                               " and add_entry are inconsistent", __FILE__, __LINE__);
    dpdfile2 *file2;
    dpdbuf4 *buf4;
    Vector *vector;
    Matrix *matrix;
    double *array;
    va_list args;
    va_start(args, numQuantities);
    double *errorVectorPtr = new double [_errorVectorSize];
    double *vectorPtr      = new double [_vectorSize];
    double *arrayPtr = errorVectorPtr;
    for(int i = 0; i < numQuantities; ++i) {
        DIISEntry::InputType type = _componentTypes[i];
        // If we've filled the error vector, start filling the vector
        if(i == _numErrorVectorComponents) arrayPtr = vectorPtr;
        switch(type){
            case DIISEntry::Pointer:
                array = va_arg(args, double*);
                for(int i = 0; i < _componentSizes[i]; ++i)
                    *arrayPtr++ = array[i];
                break;
            case DIISEntry::DPDBuf4:
                buf4 = va_arg(args, dpdbuf4*);
                for(int h = 0; h < buf4->params->nirreps; ++h){
                    dpd_buf4_mat_irrep_init(buf4, h);
                    dpd_buf4_mat_irrep_rd(buf4, h);
                    for(int row = 0; row < buf4->params->rowtot[h]; ++row){
                        for(int col = 0; col < buf4->params->coltot[h]; ++col){
                            *arrayPtr++ = buf4->matrix[h][row][col];
                        }
                    }
                    dpd_buf4_mat_irrep_close(buf4, h);
                }
                break;
            case DIISEntry::DPDFile2:
                file2 = va_arg(args, dpdfile2*);
                dpd_file2_mat_init(file2);
                dpd_file2_mat_rd(file2);
                for(int h = 0; h < file2->params->nirreps; ++h){
                    for(int row = 0; row < file2->params->rowtot[h]; ++row){
                        for(int col = 0; col < file2->params->coltot[h]; ++col){
                            *arrayPtr++ = file2->matrix[h][row][col];
                        }
                    }
                }
                dpd_file2_mat_close(file2);
                break;
            case DIISEntry::Matrix:
                matrix = va_arg(args, Matrix*);
                for(int h = 0; h < matrix->nirreps(); ++h){
                    for(int row = 0; row < matrix->rowspi()[h]; ++row){
                        for(int col = 0; col < matrix->colspi()[h]; ++col){
                            *arrayPtr++ = matrix->get(h, row, col);
                        }
                    }
                }
                break;
            case DIISEntry::Vector:
                vector = va_arg(args, Vector*);
                for(int h = 0; h < matrix->nirreps(); ++h){
                    for(int row = 0; row < vector->dimpi()[h]; ++row){
                            *arrayPtr++ = vector->get(h, row);
                    }
                }
                break;
            default:
                throw SanityCheckError("Unknown input type", __FILE__, __LINE__);
        }
    }
    va_end(args);

    int entryID = get_next_entry_id();
    if(_subspace.size() < _maxSubspaceSize){
        _subspace.push_back(new DIISEntry(_label, entryID, _entryCount++,
                            _errorVectorSize, errorVectorPtr,
                            _vectorSize, vectorPtr));
    }else{
        delete _subspace[entryID];
        _subspace[entryID] = new DIISEntry(_label, entryID, _entryCount++,
                                           _errorVectorSize, errorVectorPtr,
                                           _vectorSize, vectorPtr);
    }

    if(_storagePolicy == OnDisk) {
        _subspace[entryID]->dump_vector_to_disk();
        _subspace[entryID]->dump_error_vector_to_disk();
    }

    // Make we don't know any inner products involving this new entry
    for(int i = 0; i < _subspace.size(); ++i)
        if(i != entryID) _subspace[i]->invalidate_dot(entryID);

    return true;
}

/**
 * Figures out the ID of the next entry to be added by determining whether an entry
 * must be removed in order to add a new one.
 */
int
DIISManager::get_next_entry_id()
{
    int entry = 0;
    if(_subspace.size() < _maxSubspaceSize){
        entry = _subspace.size();
    }else{
        if(_removalPolicy == OldestAdded){
            int oldest = _subspace[0]->orderAdded();
            for(int i = 1; i < _subspace.size(); ++i){
                if(_subspace[i]->orderAdded() < oldest){
                    oldest = _subspace[i]->orderAdded();
                    entry = i;
                }
            }
        }else if (_removalPolicy == LargestError){
            double largest = _subspace[0]->rmsError();
            for(int i = 1; i < _subspace.size(); ++i){
                if(_subspace[i]->rmsError() > largest){
                    largest = _subspace[i]->rmsError();
                    entry = i;
                }
            }
        }else{
            throw SanityCheckError("Unknown RemovalPolicy", __FILE__, __LINE__);
        }
    }
    return entry;
}


/**
 * Performs the extapolation, based on the current subspace
 * @param numQuantitites - the number of quantities that the vector comprises.
 * Then pass these quantities in the order they were passed to the set_vector_size()
 * function.  The types of each component should not be specified here, unlike the
 * set_vector_size() function call.
 */
void
DIISManager::extrapolate(int numQuantities, ...)
{
    if(!_subspace.size()) return;
    int dimension = _subspace.size() + 1;
    double **bMatrix = block_matrix(dimension, dimension);
    double *coefficients = init_array(dimension);
    int *pivots = init_int_array(dimension);

    for(int i = 0; i < _subspace.size(); ++i){
        coefficients[i] = 0.0;
        bMatrix[i][_subspace.size()] = bMatrix[_subspace.size()][i] = 1.0;
        DIISEntry *entryI = _subspace[i];
        for(int j = 0; j < _subspace.size(); ++j){
            DIISEntry *entryJ = _subspace[j];
            if(entryI->dot_is_known_with(j)){
                bMatrix[i][j] = entryI->dot_with(j);
            }else{
                double dot = C_DDOT(_errorVectorSize,
                                    const_cast<double*>(entryI->errorVector()), 1,
                                    const_cast<double*>(entryJ->errorVector()), 1);
                bMatrix[i][j] = dot;
                entryI->set_dot_with(j, dot);
                entryJ->set_dot_with(i, dot);
                if(_storagePolicy == OnDisk){
                    entryI->free_error_vector_memory();
                    entryJ->free_error_vector_memory();
                }
            }
        }
    }
    coefficients[_subspace.size()] = 1.0;
    bMatrix[_subspace.size()][_subspace.size()] = 0.0;

    C_DGESV(dimension, 1, bMatrix[0], dimension, pivots, coefficients, dimension);

    dpdfile2 *file2;
    dpdbuf4 *buf4;
    Vector *vector;
    Matrix *matrix;
    double *array;
    va_list args;
    for(int n = 0; n < _subspace.size(); ++n){
        double coefficient = coefficients[n];
        const double *arrayPtr = _subspace[n]->vector();
        va_start(args, numQuantities);
        for(int i=0; i < numQuantities; ++i) {
            // The indexing arrays contain the error vector, then the vector, so they
            // need to be offset by the number of components in the error vector
            int componentIndex = i + _numErrorVectorComponents;
            DIISEntry::InputType type = _componentTypes[componentIndex];
            switch(type){
                case DIISEntry::Pointer:
                    array = va_arg(args, double*);
                    if(!n) ::memset(array, 0, _componentSizes[componentIndex] * sizeof(double));
                    for(int i = 0; i < _componentSizes[componentIndex]; ++i)
                        array[i] += coefficient * *arrayPtr++;
                    break;
                case DIISEntry::DPDBuf4:
                    buf4 = va_arg(args, dpdbuf4*);
                    if(!n) dpd_buf4_scm(buf4, 0.0);
                    for(int h = 0; h < buf4->params->nirreps; ++h){
                        dpd_buf4_mat_irrep_init(buf4, h);
                        dpd_buf4_mat_irrep_rd(buf4, h);
                        for(int row = 0; row < buf4->params->rowtot[h]; ++row){
                            for(int col = 0; col < buf4->params->coltot[h]; ++col){
                                buf4->matrix[h][row][col] += coefficient * *arrayPtr++;
                            }
                        }
                        dpd_buf4_mat_irrep_wrt(buf4, h);
                        dpd_buf4_mat_irrep_close(buf4, h);
                    }
                    break;
                case DIISEntry::DPDFile2:
                    file2 = va_arg(args, dpdfile2*);
                    if(!n) dpd_file2_scm(file2, 0.0);
                    dpd_file2_mat_init(file2);
                    dpd_file2_mat_rd(file2);
                    for(int h = 0; h < file2->params->nirreps; ++h){
                        for(int row = 0; row < file2->params->rowtot[h]; ++row){
                            for(int col = 0; col < file2->params->coltot[h]; ++col){
                                 file2->matrix[h][row][col] += coefficient * *arrayPtr++;
                            }
                        }
                    }
                    dpd_file2_mat_wrt(file2);
                    dpd_file2_mat_close(file2);
                    break;
                case DIISEntry::Matrix:
                    matrix = va_arg(args, Matrix*);
                    if(!n) matrix->zero();
                    for(int h = 0; h < matrix->nirreps(); ++h){
                        for(int row = 0; row < matrix->rowspi()[h]; ++row){
                            for(int col = 0; col < matrix->colspi()[h]; ++col){
                                matrix->add(h, row, col, coefficient * *arrayPtr++);
                            }
                        }
                    }
                    break;
                case DIISEntry::Vector:
                    vector = va_arg(args, Vector*);
                    if(!n){
                        for(int h = 0; h < matrix->nirreps(); ++h){
                            for(int row = 0; row < vector->dimpi()[h]; ++row){
                                vector->set(h, row, 0.0);
                            }
                        }
                    }
                    for(int h = 0; h < matrix->nirreps(); ++h){
                        for(int row = 0; row < vector->dimpi()[h]; ++row){
                            double val = vector->get(h, row);
                            vector->set(h, row, coefficient * *arrayPtr++ + val);
                        }
                    }
                    break;
                default:
                    throw SanityCheckError("Unknown input type", __FILE__, __LINE__);
            }
        }
        if(_storagePolicy == OnDisk) _subspace[n]->free_vector_memory();
        va_end(args);
    }
    free_block(bMatrix);
}


DIISManager::~DIISManager()
{
    if (_default_psio_lib_->open_check(PSIF_LIBDIIS))
        _default_psio_lib_->close(PSIF_LIBDIIS, 1);
}

} // Namespace