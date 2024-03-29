#include "pycsparse.h"
#include "minicore/hash/hash.h"

template<typename FT, template<typename> class Hasher>
auto project_array(const Hasher<FT> &hasher, const PyCSparseMatrix &smw, bool round=false) -> py::object {
    const ssize_t nh = hasher.nh();
    const ssize_t nr = smw.rows();
    py::array_t<FT> rv(
        py::buffer_info(nullptr, sizeof(FT), py::format_descriptor<FT>::format(),
                        2,
                        std::vector<Py_ssize_t>{nr, nh}, // shape
                        // strides
                        std::vector<Py_ssize_t>{Py_ssize_t(sizeof(FT) * nh),
                                                Py_ssize_t(sizeof(FT))}
        )
    );
    auto rvinf = rv.request();
    const char smwt = std::tolower(smw.indptr_t_[0]), smwi = std::tolower(smw.indices_t_[0]);
    for(size_t i = 0; i < smw.rows(); ++i) {
        ssize_t start, stop;
        if(smwt == 'i') {
            start = ((uint32_t *)smw.indptrp_)[i];
            stop = ((uint32_t *)smw.indptrp_)[i + 1];
        } else if(smwt == 'b') {
            start = ((uint8_t *)smw.indptrp_)[i];
            stop = ((uint8_t *)smw.indptrp_)[i + 1];
        } else if(smwt == 'h') {
            start = ((uint16_t *)smw.indptrp_)[i];
            stop = ((uint16_t *)smw.indptrp_)[i + 1];
        } else if(smwt == 'l') {
            start = ((uint64_t *)smw.indptrp_)[i];
            stop = ((uint64_t *)smw.indptrp_)[i + 1];
        } else throw std::invalid_argument("Wrong datatype for indptr");
        FT *retp = (FT *)rvinf.ptr + (i * nh);
        auto nnz = stop - start;
        switch(smw.data_t_[0]) {
            case 'B':
                if(smwi == 'i') hasher.project((uint8_t *)smw.datap_ + start, (uint32_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'l') hasher.project((uint8_t *)smw.datap_ + start, (uint64_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'h') hasher.project((uint8_t *)smw.datap_ + start, (uint16_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'b') hasher.project((uint8_t *)smw.datap_ + start, (uint8_t *)smw.indicesp_ + start, nnz, retp);
                else throw std::invalid_argument("Wrong datatype for indices");
                break;
            case 'H':
                if(smwi == 'i') hasher.project((uint16_t *)smw.datap_ + start, (uint32_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'l') hasher.project((uint16_t *)smw.datap_ + start, (uint64_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'h') hasher.project((uint16_t *)smw.datap_ + start, (uint16_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'b') hasher.project((uint16_t *)smw.datap_ + start, (uint8_t *)smw.indicesp_ + start, nnz, retp);
                else throw std::invalid_argument("Wrong datatype for indices");
                break;
            case 'i': case 'I':
                if(smwi == 'i') hasher.project((uint32_t *)smw.datap_ + start, (uint32_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'l') hasher.project((uint32_t *)smw.datap_ + start, (uint64_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'h') hasher.project((uint32_t *)smw.datap_ + start, (uint16_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'b') hasher.project((uint32_t *)smw.datap_ + start, (uint8_t *)smw.indicesp_ + start, nnz, retp);
                else throw std::invalid_argument("Wrong datatype for indices");
                break;
            case 'd':
                if(smwi == 'i') hasher.project((double *)smw.datap_ + start, (uint32_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'l') hasher.project((double *)smw.datap_ + start, (uint64_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'h') hasher.project((double *)smw.datap_ + start, (uint16_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'b') hasher.project((double *)smw.datap_ + start, (uint8_t *)smw.indicesp_ + start, nnz, retp);
                else throw std::invalid_argument("Wrong datatype for indices");
                break;
            case 'f':
                if(smwi == 'i') hasher.project((float *)smw.datap_ + start, (uint32_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'l') hasher.project((float *)smw.datap_ + start, (uint64_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'h') hasher.project((float *)smw.datap_ + start, (uint16_t *)smw.indicesp_ + start, nnz, retp);
                else if(smwi == 'b') hasher.project((float *)smw.datap_ + start, (uint8_t *)smw.indicesp_ + start, nnz, retp);
                else throw std::invalid_argument("Wrong datatype for indices");
                break;
            default: throw std::invalid_argument("Wrong datatype for indices");
            }
        if(round) {
            auto view = blz::make_cv((FT *)rvinf.ptr, nr * nh);
            view = floor(view);
        }
    }
    return rv;
}



template<typename FT, template<typename> class Hasher>
auto project_array(const Hasher<FT> &hasher, const py::array obj, bool round=false) {
    auto bi = obj.request();
    auto nd = bi.ndim;
    py::object ret = py::none();
    FT *ptr = (FT *)bi.ptr;
    const ssize_t nh = hasher.nh();
    if(bi.itemsize != sizeof(FT)) throw std::invalid_argument("Sanity check: itemsize and type size are different; You're using a " + std::string(std::is_same_v<FT, float> ? "float": std::is_same_v<FT, double> ? "double": "unknown") + "hasher but hashing items of a different size. JSDLSHasher hashes floats, but JSDLSHasher_d hashes doubles. You must select the same precision.");
    if((std::is_same_v<FT, double> && bi.format.front() != 'd') || (std::is_same_v<FT, float> && bi.format.front() != 'f'))
        throw std::invalid_argument(std::string("Type of array ") + bi.format + " does not match hasher with items of size " + std::to_string(bi.itemsize));
    if(nd == 1) {
        py::array_t<FT> rv(nh);
        auto rvb = rv.request();
        blaze::CustomVector<FT, blaze::unaligned, blaze::unpadded> cv((FT *)rvb.ptr, nh);
        blaze::CustomVector<FT, blaze::unaligned, blaze::unpadded> dv(ptr, bi.size);
        if(round)
            cv = hasher.hash(dv);
        else
            cv = hasher.project(dv);
        ret = rv;
    } else if(nd == 2) {
        const ssize_t nr = bi.shape[0];
        py::array_t<FT> rv(
            py::buffer_info(nullptr, sizeof(FT), py::format_descriptor<FT>::format(),
                            2,
                            std::vector<Py_ssize_t>{nr, nh}, // shape
                            // strides
                            std::vector<Py_ssize_t>{Py_ssize_t(sizeof(FT) * nh),
                                                    Py_ssize_t(sizeof(FT))}
            )
        );
        auto rvb = rv.request();
        blaze::CustomMatrix<FT, blaze::unaligned, blaze::unpadded> cm((FT *)rvb.ptr, nr, nh);
        blaze::CustomMatrix<FT, blaze::unaligned, blaze::unpadded> dm(ptr, nr, bi.shape[1]);
        if(round) cm = hasher.hash(dm);
        else      cm = hasher.project(dm);
        ret = rv;
    } else {
        throw std::invalid_argument("Wrong number of dimensions (must be 1 or 2)");
    }
    return ret;
}

template<typename FT, template<typename> class Hasher>
py::object project_array(const Hasher<FT> &hasher, py::object obj, bool round=false) {
    if(py::isinstance<py::array>(obj))
        return project_array(hasher, obj.cast<py::array>(), round);
    if(py::isinstance<PyCSparseMatrix>(obj)) {
        return project_array(hasher, obj.cast<PyCSparseMatrix>(), round);
    }
    return py::none();
}


#if 0
template<typename FT, template<typename> class Hasher>
auto project_array(const Hasher<FT> &hasher, const SparseMatrixWrapper &smw, bool round=false) -> py::object {
    const ssize_t nh = hasher.nh();
    const ssize_t nr = smw.rows();
    py::array_t<FT> rv(
        py::buffer_info(nullptr, sizeof(FT), py::format_descriptor<FT>::format(),
                        2,
                        std::vector<Py_ssize_t>{nr, nh}, // shape
                        // strides
                        std::vector<Py_ssize_t>{Py_ssize_t(sizeof(FT) * nh),
                                                Py_ssize_t(sizeof(FT))}
        )
    );
    auto rvb = rv.request();
    blaze::CustomMatrix<FT, blaze::unaligned, blaze::unpadded> cm((FT *)rvb.ptr, nr, nh);
    smw.perform([&](const auto &x){if(round) cm = hasher.hash(x); else cm = hasher.project(x);});
    return rv;
}
#endif

#define SETTINGS_GETTER(type, ftype) def("settings", [](const type<ftype> &hasher) -> LSHasherSettings {return hasher.settings_;}, "Get settings struct from hasher")
#if 1
#define SMW_HASH_DEC(type, ftype) .def("hash", [](const type<ftype> &hasher, const PyCSparseMatrix &smw) {return project_array(hasher, smw, true);}) \
                                 .def("project", [](const type<ftype> &hasher, const PyCSparseMatrix &smw) {return project_array(hasher, smw, false);})
#else
#define SMW_HASH_DEC(type, ftype)
#endif

void init_hashers(py::module &m) {
    py::class_<LSHasherSettings>(m, "LSHSettings")
        .def(py::init<unsigned, unsigned, unsigned>(), py::arg("dim"), py::arg("k"), py::arg("l"))
        .def_readwrite("dim_", &LSHasherSettings::dim_)
        .def_readwrite("k_", &LSHasherSettings::k_)
        .def_readwrite("l_", &LSHasherSettings::l_)
        .def("nhashes", [](const LSHasherSettings &x) -> int {return x.k_ * x.l_;})
        .def("__eq__", [](const LSHasherSettings &lh, const LSHasherSettings &rh) {return lh == rh;})
        .def("__ne__", [](const LSHasherSettings &lh, const LSHasherSettings &rh) {return lh != rh;});

    py::class_<JSDLSHasher<double>>(m, "JSDLSHasher_d")
        .def(py::init<unsigned, unsigned, unsigned, double, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("r") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, double, uint64_t>(), py::arg("settings"), py::arg("r") = .1, py::arg("seed") = 0)
        .def("project", [](const JSDLSHasher<double> &hasher, py::object obj) -> py::object {
            return project_array(hasher, obj);
        })
        .def("hash", [](const JSDLSHasher<double> &hasher, py::object obj) -> py::object {
            return project_array(hasher, obj, true);
        })
        .SETTINGS_GETTER(JSDLSHasher, double)
        SMW_HASH_DEC(JSDLSHasher, double);
    py::class_<S2JSDLSHasher<double>>(m, "S2JSDLSHasher_d")
        .def(py::init<unsigned, unsigned, unsigned, double, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, double, uint64_t>(), py::arg("settings"), py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const S2JSDLSHasher<double> &hasher, py::object obj) {
            return project_array(hasher, obj);
        }).SETTINGS_GETTER(S2JSDLSHasher, double)
        SMW_HASH_DEC(S2JSDLSHasher, double)
        .def("hash", [](const S2JSDLSHasher<double> &hasher, py::object obj) {return project_array(hasher, obj, true);});
    py::class_<L1LSHasher<double>>(m, "L1LSHasher_d")
        .def(py::init<unsigned, unsigned, unsigned, double, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, double, uint64_t>(), py::arg("settings"), py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const L1LSHasher<double> &hasher, py::object obj) {
            return project_array(hasher, obj);
        }).SETTINGS_GETTER(L1LSHasher, double)
        SMW_HASH_DEC(L1LSHasher, double)
        .def("hash", [](const L1LSHasher<double> &hasher, py::object obj) {return project_array(hasher, obj, true);});
    py::class_<ClippedL1LSHasher<double>>(m, "ClippedL1LSHasher_d")
        .def(py::init<unsigned, unsigned, unsigned, double, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, double, uint64_t>(), py::arg("settings"), py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const ClippedL1LSHasher<double> &hasher, py::object obj) {
            return project_array(hasher, obj);
        }).SETTINGS_GETTER(ClippedL1LSHasher, double)
        .def("hash", [](const ClippedL1LSHasher<double> &hasher, py::object obj) {return project_array(hasher, obj, true);})
        SMW_HASH_DEC(ClippedL1LSHasher, double);
    py::class_<L2LSHasher<double>>(m, "L2LSHasher_d")
        .def(py::init<unsigned, unsigned, unsigned, double, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, double, uint64_t>(), py::arg("settings"), py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const L2LSHasher<double> &hasher, py::object obj) {
            return project_array(hasher, obj);
         }).SETTINGS_GETTER(L2LSHasher, double)
        SMW_HASH_DEC(L2LSHasher, double)
        .def("hash", [](const L2LSHasher<double> &hasher, py::object obj) {return project_array(hasher, obj, true);});
    py::class_<LpLSHasher<double>>(m, "LpLSHasher_d")
        .def(py::init<unsigned, unsigned, unsigned, double, double, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("p")=1.1, py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, double, double, uint64_t>(), py::arg("settings"), py::arg("p")=1.1, py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const LpLSHasher<double> &hasher, py::object obj) {return project_array(hasher, obj);})
        .SETTINGS_GETTER(L2LSHasher, double)
        SMW_HASH_DEC(LpLSHasher, double)
        .def("hash", [](const LpLSHasher<double> &hasher, py::object obj) {return project_array(hasher, obj, true);});
// To undo this, uncomment the above and suffix the hashers below with _f to signify use of floats
// To save compilation time, we're only allowing floats
    py::class_<JSDLSHasher<float>>(m, "JSDLSHasher")
        .def(py::init<unsigned, unsigned, unsigned, float, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("r") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, float, uint64_t>(), py::arg("settings"), py::arg("r") = .1, py::arg("seed") = 0)
        .def("project", [](const JSDLSHasher<float> &hasher, py::array obj) -> py::object {
            return project_array(hasher, obj, false);
        })
        .def("hash", [](const JSDLSHasher<float> &hasher, py::array obj) -> py::object {
            return project_array(hasher, obj, true);
        })
        .SETTINGS_GETTER(JSDLSHasher, float)
        SMW_HASH_DEC(JSDLSHasher, float);
    py::class_<S2JSDLSHasher<float>>(m, "S2JSDLSHasher")
        .def(py::init<unsigned, unsigned, unsigned, float, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, float, uint64_t>(), py::arg("settings"), py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const S2JSDLSHasher<float> &hasher, py::array obj) {
            return project_array(hasher, obj, false);
        })
        .def("hash", [](const S2JSDLSHasher<float> &hasher, py::array obj) {
            return project_array(hasher, obj, true);
        })
        .SETTINGS_GETTER(S2JSDLSHasher, float)
        SMW_HASH_DEC(S2JSDLSHasher, float);
    py::class_<L1LSHasher<float>>(m, "L1LSHasher")
        .def(py::init<unsigned, unsigned, unsigned, float, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, float, uint64_t>(), py::arg("settings"), py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const L1LSHasher<float> &hasher, py::array obj) {
            return project_array(hasher, obj, false);
        }).SETTINGS_GETTER(L1LSHasher, float)
        .def("hash", [](const L1LSHasher<float> &hasher, py::array obj) {
            return project_array(hasher, obj, true);
        })
        SMW_HASH_DEC(L1LSHasher, float);
    py::class_<ClippedL1LSHasher<float>>(m, "ClippedL1LSHasher")
        .def(py::init<unsigned, unsigned, unsigned, float, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, float, uint64_t>(), py::arg("settings"), py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const ClippedL1LSHasher<float> &hasher, py::array obj) {
            return project_array(hasher, obj, false);
        })
        .def("hash", [](const ClippedL1LSHasher<float> &hasher, py::array obj) {
            return project_array(hasher, obj, true);
        })
        .SETTINGS_GETTER(ClippedL1LSHasher, float)
        SMW_HASH_DEC(ClippedL1LSHasher, float);
    py::class_<L2LSHasher<float>>(m, "L2LSHasher")
        .def(py::init<unsigned, unsigned, unsigned, float, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, float, uint64_t>(), py::arg("settings"), py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const L2LSHasher<float> &hasher, py::array obj) {
            return project_array(hasher, obj, false);
         })
        .def("hash", [](const L2LSHasher<float> &hasher, py::array obj) {
            return project_array(hasher, obj, true);
         }).SETTINGS_GETTER(L2LSHasher, float)
        SMW_HASH_DEC(L2LSHasher, float);
    py::class_<LpLSHasher<float>>(m, "LpLSHasher")
        .def(py::init<unsigned, unsigned, unsigned, float, float, uint64_t>(), py::arg("dim"), py::arg("k"), py::arg("l"), py::arg("p")=1.1, py::arg("w") = .1, py::arg("seed") = 0)
        .def(py::init<LSHasherSettings, float, float, uint64_t>(), py::arg("settings"), py::arg("p")=1.1, py::arg("w") = .1, py::arg("seed") = 0)
        .def("project", [](const LpLSHasher<float> &hasher, py::array obj) {return project_array(hasher, obj);})
        .SETTINGS_GETTER(LpLSHasher, float)
        SMW_HASH_DEC(LpLSHasher, float);
}

#if 0
template<typename VT, typename IT, typename IPtrT, typename FT=float>
py::object csr_msr_2proj(
    const CSparseMatrix<VT, IT, IPtrT> &mat,
    dist::DissimilarityMeasure msr,
    int k, int l_nhashes=k * 2, int k_nprojperhash=1, int maxdistperp=k * 3, double p=0.)
{
    py::object ret;
    if(l_nhashes < 0 || k_nprojperhash < 0 || maxdistperp < 0) throw std::invalid_argument("nhashes, nprojperhash, or maxdistperp are < 0.");
    // Step 1: compute projections
    if(msr == distance::L1 || msr == distance::L2 || msr == distance::SQRL2 || p > 0.) {
        // Project
    } else if(msr == distance::JSD || msr == distance::S2JSD || msr == distance::UWLLR ||
       msr == distance::LLR || msr == distance::MKL || msr == distance::REVERSE_MKL ||
       msr == distance::ITAKURA_SAITO || msr == distance::REVERSE_ITAKURA_SAITO ||
       msr == distance::HELLINGER || msr == distance::BHATTACHARYYA_DISTANCE || msr == distance::TVD ||
       msr == distance::SRLRT || msr == distance::SRULRT || msr == distance::POISSON)
    {
        blz::DV<double> rsis = 1. / blaze::generate(mat.rows(), [&mat](auto x) {return sum(row(mat, x));});
        py::array arr(py::dtype("f"), std::vector<py::ssize_t>{{py::ssize_t(mat.rows()), py::ssize_t(mat.columns())}});
        ret = arr;
       // Compute row sums, then use normalized JSD LSH (for everything but S2JSD)
    }
    return ret;
}
    // Step 2: Take projections, then aggregate into subkeys and build index
    //         Choice: use LSH table or DCI
    // Step 3: Use the oracle to generate potential nearest neighbors
    // Step 4: create the list of nearest neighbors
#endif

#undef SETTINGS_GETTER
#undef SMW_HASH_DEC
PYBIND11_MODULE(minilsh, m) {
    init_hashers(m);
    m.doc() = "Python bindings for LSH functions";
}
