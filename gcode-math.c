/*
 ============================================================================
 Name        : gcode-math.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-16)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Coordinate System Transformations Code
 ============================================================================
 */

#include "gcode-math.h"
#include "gcode-commons.h"
#include "gcode-parameters.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>


double do_G_coordinate_math(const TGCodeCoordinateInfo *system, double input,
    const double offset, const double previous, const uint8_t axis) {
  if(!isnan(input))
    return inch_math(
        relative_math(
            system_math(
                input, (system->current == GCODE_MCS), offset,
                fetch_parameter(
                    GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) *
                    GCODE_PARM_WCS_SIZE + axis)),
            previous, (system->absolute == GCODE_ABSOLUTE)),
        (system->units == GCODE_UNITS_INCH));
  else return previous;
}

double current_or_last_math(double input, double last) {
  if(isnan(input))
    return last;
  else
    return input;
}

double current_or_zero_math(double value, double last, bool absolute,
    bool missing) {
  if(missing)
    if(absolute)
      return last;
    else
      return +0.0E+0;
  else
    return value;
}

double relative_math(double input, double origin, bool absolute) {
    if(absolute)
      return input;
    else
      return input + origin;
}

double system_math(double input, bool MCS, double offset, double origin) {
  if(MCS)
    return input;
  else
    return origin + offset + input;
}

double length_comp_math(double input, TGCodeCompSpec comp) {
  if(comp.mode == GCODE_COMP_LEN_OFF)
    return input;
  else
    if(comp.mode == GCODE_COMP_LEN_P)
      return input + comp.offset;
    else
      return input - comp.offset;
}

double inch_math(double input, bool inch) {
  if(inch)
    return input * GCODE_INCH2MM;
  else
    return input;
}

void polar_math(double radius, double theta, double *X, double *Y) {
  *X = radius * cos(theta * GCODE_DEG2RAD);
  *Y = radius * sin(theta * GCODE_DEG2RAD);
}

void rotation_math(double inputX, double inputY, double theta, double originX,
    double originY, double *X, double *Y) {
  *X = cos(theta * GCODE_DEG2RAD) * (inputX - originX) -
      sin(theta * GCODE_DEG2RAD) * (inputY - originY) + originX;
  *Y = sin(theta * GCODE_DEG2RAD) * (inputX - originX) +
      cos(theta * GCODE_DEG2RAD) * (inputY - originY) + originY;
}

double scaling_math(double input, double origin, double factor) {
  return origin + (input - origin) * factor;
}

double mirroring_math(double input, double previous, double *original, bool mirrored) {
  if(mirrored) {
      previous -= input - *original;
      *original = input;
    } else previous = input;

  return previous;
}

void arc_math(double X, double Y, double oldX, double oldY, double *R,
    double *I, double *J, double *K, bool invert) {
  if(!isnan(*R)) {
    double d = hypot(oldX - X, oldY - Y);
    *I = (X - oldX) / 2 + (invert ? -1 : 1) * sqrt(*R * *R - d * d / 4) * (Y - oldY) / d;
    *J = (Y - oldY) / 2 + (invert ? 1 : -1) * sqrt(*R * *R - d * d / 4) * (X - oldX) / d;
    *K = 0;
  } else *R = hypot(*I, *J);
}

void move_math(TGCodeCoordinateInfo *system, double X, double Y, double Z) {
  TGCodeAbsoluteMode oldAbsolute;
  double newX, newY, newZ, newrX, newrY, newrZ, newcX, newcY, newcZ;

  system->cX = current_or_last_math(X, system->cX);
  system->cY = current_or_last_math(Y, system->cY);
  system->cZ = current_or_last_math(Z, system->cZ);
  /* c[XYZ] now all contain non-NAN taken either from the current block or
   * the previous word address value.
   *
   * NOTE: this is the end of processing for c[XYZ]: they're meant to contain
   *       the word address values from the last block. */

  if(system->cartesian == GCODE_POLAR) {
    polar_math(system->cX, system->cY, &newcX, &newcY);
    oldAbsolute = system->absolute;
    system->absolute = GCODE_RELATIVE;
    /* newc[XY] now contain the Cartesian equivalent of what was specified in
     * polar coordinates in the current block. Since polar coordinates always
     * work in incremental mode, we temporarily change to that to match. */
  } else {
    newcX = current_or_zero_math(
        system->cX, system->cX, (system->absolute == GCODE_ABSOLUTE), isnan(X));
    newcY = current_or_zero_math(
        system->cY, system->cY, (system->absolute == GCODE_ABSOLUTE), isnan(Y));
  }
  newcZ = current_or_zero_math(
      system->cZ, system->cZ, (system->absolute == GCODE_ABSOLUTE), isnan(Z));
  /* newc[XYZ] now contain the input value for all calculations below */

  system->gX = relative_math(newcX, system->gX,
                        (system->absolute == GCODE_ABSOLUTE));
  system->gY = relative_math(newcY, system->gY,
                        (system->absolute == GCODE_ABSOLUTE));
  system->gZ = relative_math(newcZ, system->gZ,
                             (system->absolute == GCODE_ABSOLUTE));
  /* g[XYZ] now contain the relative-corrected version of c[XYZ] as specified
   * in the current block or inferred from past state */

  if(system->cartesian == GCODE_POLAR)
    /* Restore previous state and word address contents if we were in polar */
    system->absolute = oldAbsolute;

  system->gX = system_math(
      system->gX, (system->current == GCODE_MCS), system->offset.X,
      fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) *
                      GCODE_PARM_WCS_SIZE + GCODE_AXIS_X));
  system->gY = system_math(
      system->gY, (system->current == GCODE_MCS), system->offset.Y,
      fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) *
                      GCODE_PARM_WCS_SIZE + GCODE_AXIS_Y));
  system->gZ = system_math(
      system->gZ, (system->current == GCODE_MCS), system->offset.Z,
      fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) *
                      GCODE_PARM_WCS_SIZE + GCODE_AXIS_Z));
  /* g[XYZ] now contain the MCS-, WCS- and LCS- corrected version of their
   * previous self */

  system->gZ = length_comp_math(system->gZ, system->lenComp);
  /* g[XYZ] now contain the length-compensated version of their previous self.
   * NOTE: compensation is dimension-less, as per the standard.
   * NOTE: this is the end of processing for g[XYZ]: they're meant to contain
   *       the G-Code interpreter's idea of the current coordinates */

  newX = inch_math(system->gX, (system->units == GCODE_UNITS_INCH));
  newY = inch_math(system->gY, (system->units == GCODE_UNITS_INCH));
  newZ = inch_math(system->gZ, (system->units == GCODE_UNITS_INCH));
  /* new[XYZ] now contain g[XYZ] in machine units */

  if(system->rotation.mode == GCODE_ROTATION_ON) {
    switch(system->plane) {
      case GCODE_PLANE_XY:
        rotation_math(newX, newY, system->rotation.R, system->rotation.X,
                      system->rotation.Y, &newrX, &newrY);
        newrZ = newZ;
        break;
      case GCODE_PLANE_YZ:
        rotation_math(newY, newZ, system->rotation.R, system->rotation.Y,
                      system->rotation.Z, &newrY, &newrZ);
        newrX = newX;
        break;
      case GCODE_PLANE_ZX:
        rotation_math(newZ, newX, system->rotation.R, system->rotation.Z,
                      system->rotation.X, &newrZ, &newrX);
        newrY = newY;
        break;
    }
    newX = newrX;
    newY = newrY;
    newZ = newrZ;
  }
  /* new[XYZ] now contain the rotated version of new[XYZ] according to the
   * current coordinate system rotation mode and parameters and active plane */

  if(system->scaling.mode == GCODE_SCALING_ON) {
    newX = scaling_math(newX, system->scaling.X, system->scaling.I);
    newY = scaling_math(newY, system->scaling.Y, system->scaling.J);
    newZ = scaling_math(newZ, system->scaling.Z, system->scaling.K);
  }
  /* new[XYZ] now contain the scaled version of newr[XYZ] according to the
   * current scaling mode and parameters */

  /* done, copy over to machine coordinates */
  system->X = newX;
  system->Y = newY;
  system->Z = newZ;
}

TGCodeRadCompMode vector_side_math(double x1, double y1, double x2, double y2,
    double x3, double y3) {
  double side = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);

  if(fpclassify(side) == FP_ZERO)
    return GCODE_COMP_RAD_OFF;
  else if(signbit(side))
    return GCODE_COMP_RAD_R;
  else
    return GCODE_COMP_RAD_L;
}

TGCodeMoveSpec offset_math(TGCodeMoveSpec pM, TGCodeMoveSpec tM,
    TGCodeCompSpec radComp, double *originX, double *originY) {
  double invert;

  /* Do we actually have anything to do here? */
  if(radComp.mode == GCODE_COMP_RAD_OFF) return tM;

  if(tM.isArc) {
    double sAngle = atan2(pM.target.Y - tM.center.Y,
                          pM.target.X - tM.center.X) * GCODE_RAD2DEG;
    double eAngle = atan2(tM.target.Y - tM.center.Y,
                          tM.target.X - tM.center.X) * GCODE_RAD2DEG;
    double radius = hypot(tM.center.X - tM.target.X, tM.center.Y - tM.target.Y);
    TGCodeRadCompMode cside;

    if(signbit(sAngle - eAngle))
      if(tM.ccw)
        invert = -1.0;
      else
        invert = +1.0;
    else
      if(tM.ccw)
        invert = +1.0;
      else
        invert = -1.0;

    if(round(fabs(sAngle - eAngle)) == 180)
      if(tM.ccw)
        cside = GCODE_COMP_RAD_L;
      else
        cside = GCODE_COMP_RAD_R;
    else
      /* Draw a chord from start to finish and check the side the center falls on. */
      cside = vector_side_math(pM.target.X, pM.target.Y, tM.target.X,
                               tM.target.Y, tM.center.X, tM.center.Y);

    if(cside == radComp.mode)
      radius -= radComp.offset * invert;
    else
      radius += radComp.offset * invert;

    *originX = radius * cos(sAngle * GCODE_DEG2RAD);
    *originY = radius * sin(sAngle * GCODE_DEG2RAD);
    tM.target.X = radius * cos(eAngle * GCODE_DEG2RAD);
    tM.target.Y = radius * sin(eAngle * GCODE_DEG2RAD);
  } else {
    double angle = atan2(tM.target.Y - pM.target.Y,
                         tM.target.X - pM.target.X) * GCODE_RAD2DEG;
    double coefx, coefy;

    if(radComp.mode == GCODE_COMP_RAD_L)
      invert = +1.0;
    else
      invert = -1.0;

    if(angle >= 0 && angle <= 90) {
      angle = 90 - angle;
      coefx = -1.0 * invert;
      coefy = +1.0 * invert;
    } else if(angle > 90 && angle <= 180) {
      angle -= 90;
      coefx = -1.0 * invert;
      coefy = -1.0 * invert;
    } else if(angle > -180 && angle <= -90) {
      angle = -90 - angle;
      coefx = +1.0 * invert;
      coefy = -1.0 * invert;
    } else if(angle > -90 && angle < 0) {
      angle += 90;
      coefx = +1.0 * invert;
      coefy = +1.0 * invert;
    }

    *originX = pM.target.X + coefx * cos(angle * GCODE_DEG2RAD) * radComp.offset;
    *originY = pM.target.Y + coefy * sin(angle * GCODE_DEG2RAD) * radComp.offset;
    tM.target.X += coefx * cos(angle * GCODE_DEG2RAD) * radComp.offset;
    tM.target.Y += coefy * sin(angle * GCODE_DEG2RAD) * radComp.offset;
  }

  return tM;
}
