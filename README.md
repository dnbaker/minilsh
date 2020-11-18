### minilsh

Install via `MINICORE_DIR=$PATH_TO_MINICORE python3 setup.py install`.

Projects dense numpy matrices (float32), or sparse matrices in CSR notation, as wrapped by minicore.PyCSparseMatrix.

Call hasher.project() to compute projections (for dimensionality reduction or DCI) and hasher.hash() to floor these values for use in an LSH table.
