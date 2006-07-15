/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Oracle Corporation code.
 *
 * The Initial Developer of the Original Code is Oracle Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@pavlov.net>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef GFX_MATRIX_H
#define GFX_MATRIX_H

#include <cairo.h>

#include "gfxPoint.h"
#include "gfxTypes.h"
#include "gfxRect.h"

// XX - I don't think this class should use gfxFloat at all,
// but should use 'double' and be called gfxDoubleMatrix;
// we can then typedef that to gfxMatrix where we typedef
// double to be gfxFloat.

/**
 * A matrix that represents an affine transformation. Projective
 * transformations are not supported. This matrix looks like:
 *
 * / a b tx \
 * | c d ty |
 * \ 0 0  1 /
 *
 * So, transforming a point (x, y) results in:
 *
 *           / a  b  0 \   / a * x + c * y + tx \ T
 * (x y 1) * | c  d  0 | = | b * x + d * y + ty |
 *           \ tx ty 1 /   \         1          /
 *
 */
class THEBES_API gfxMatrix {
protected:
    cairo_matrix_t mat;

public:
    /**
     * Initializes this matrix as the identity matrix.
     */
    gfxMatrix() { Reset(); }
    gfxMatrix(const gfxMatrix& m) : mat(m.mat) {}
    /**
     * Initializes the matrix from individual components. See the class
     * description for the layout of the matrix.
     */
    gfxMatrix(gfxFloat a, gfxFloat b, gfxFloat c, gfxFloat d, gfxFloat tx, gfxFloat ty) {
        // XXX cairo_matrix_init?
        mat.xx = a; mat.yx = b; mat.xy = c; mat.yy = d; mat.x0 = tx; mat.y0 = ty;
    }

    gfxMatrix(const cairo_matrix_t& m) {
        mat = m;
    }

    bool operator==(const gfxMatrix& m) const {
        return ((mat.xx == m.mat.xx) &&
                (mat.yx == m.mat.yx) &&
                (mat.xy == m.mat.xy) &&
                (mat.yy == m.mat.yy) &&
                (mat.x0 == m.mat.x0) &&
                (mat.y0 == m.mat.y0));
    }

    bool operator!=(const gfxMatrix& m) const {
        return !(*this == m);
    }

    gfxMatrix& operator=(const cairo_matrix_t& m) {
        mat = m;
        return *this;
    }

    /**
     * Post-multiplies m onto the matrix.
     */
    const gfxMatrix& operator *= (const gfxMatrix& m) {
        return Multiply(m);
    }

    /**
     * Multiplies *this with m and returns the result.
     */
    gfxMatrix operator * (const gfxMatrix& m) const {
        return gfxMatrix(*this).Multiply(m);
    }

    // conversion to other types
    const cairo_matrix_t& ToCairoMatrix() const {
        return mat;
    }

    void ToValues(gfxFloat *xx, gfxFloat *yx,
                  gfxFloat *xy, gfxFloat *yy,
                  gfxFloat *x0, gfxFloat *y0) const
    {
        *xx = mat.xx;
        *yx = mat.yx;
        *xy = mat.xy;
        *yy = mat.yy;
        *x0 = mat.x0;
        *y0 = mat.y0;
    }

    // matrix operations
    /**
     * Resets this matrix to the identity matrix.
     */
    const gfxMatrix& Reset() {
        cairo_matrix_init_identity(&mat);
        return *this;
    }

    /**
     * Inverts this matrix, if possible. Otherwise, the matrix is left
     * unchanged.
     *
     * XXX should this do something with the return value of
     * cairo_matrix_invert?
     */
    const gfxMatrix& Invert() {
        cairo_matrix_invert(&mat);
        return *this;
    }

    /**
     * Scales this matrix. The scale is pre-multiplied onto this matrix,
     * i.e. the scaling takes place before the other transformations.
     */
    const gfxMatrix& Scale(gfxFloat x, gfxFloat y) {
        cairo_matrix_scale(&mat, x, y);
        return *this;
    }

    /**
     * Translates this matrix. The translation is pre-multiplied onto this matrix,
     * i.e. the translation takes place before the other transformations.
     */
    const gfxMatrix& Translate(const gfxPoint& pt) {
        cairo_matrix_translate(&mat, pt.x, pt.y);
        return *this;
    }

    /**
     * Rotates this matrix. The rotation is pre-multiplied onto this matrix,
     * i.e. the translation takes place after the other transformations.
     *
     * @param radians Angle in radians.
     */
    const gfxMatrix& Rotate(gfxFloat radians) {
        // cairo_matrix_rotate?
        gfxFloat s = sin(radians);
        gfxFloat c = cos(radians);
        gfxMatrix t( c, s,
                    -s, c,
                     0, 0);
        return *this = t.Multiply(*this);
    }

     /**
      * Multiplies the current matrix with m.
      * This is a post-multiplication, i.e. the transformations of m are
      * applied _after_ the existing transformations.
      *
      * XXX is that difference (compared to Rotate etc) a good thing?
      */
    const gfxMatrix& Multiply(const gfxMatrix& m) {
        cairo_matrix_multiply(&mat, &mat, &m.mat);
        return *this;
    }

    /**
     * Transforms a point according to this matrix.
     */
    gfxPoint Transform(const gfxPoint point) const {
        gfxPoint ret = point;
        cairo_matrix_transform_point(&mat, &ret.x, &ret.y);
        return ret;
    }

    /**
     * Transform a distance according to this matrix. This does not apply
     * any translation components.
     */
    gfxSize Transform(const gfxSize size) const {
        gfxSize ret = size;
        cairo_matrix_transform_distance(&mat, &ret.width, &ret.height);
        return ret;
    }

    /**
     * Transforms both the point and distance according to this matrix.
     */
    gfxRect Transform(const gfxRect rect) const {
        gfxRect ret(Transform(rect.pos), Transform(rect.size));
        return ret;
    }

    /**
     * Returns the translation component of this matrix.
     */
    gfxPoint GetTranslation() const {
        return gfxPoint(mat.x0, mat.y0);
    }

    /**
     * Returns true if the matrix has any transform other
     * than a straight translation
     */
    bool HasNonTranslation() const {
        return ((mat.xx != 1.0) || (mat.yy != 1.0) ||
                (mat.xy != 0.0) || (mat.yx != 0.0));
    }
};

#endif /* GFX_MATRIX_H */
