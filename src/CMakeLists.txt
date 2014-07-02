# Copyright (C) LAAS-CNRS
# Author : Elie MOUSSY
#
# This file is part of LibPF.
# LibPF is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

SET(SOURCES
  LibPF.cpp
  bspline/bspline2d.cpp
  bspline/multibspline2d.cpp
  bspline/point2d.cpp
  colorutils/calclut.cpp
  colorutils/cvtcolor.cpp
  colsegmentation/ColorBase.cpp
  colsegmentation/ColorModel.cpp
  colsegmentation/skincolormodel.cpp
  colsegmentation/skincolorsegmentation.cpp
  colsegmentation/skinregionsegmentation.cpp
  distribution/ColorDistribution.cpp
  distribution/Gaussian.cpp
  distribution/Gaussian1D.cpp
  distribution/Gaussian2D.cpp
  distribution/Gaussian3D.cpp
  distribution/Gaussian4D.cpp
  distribution/GaussianAlloc.cpp
  distribution/GaussianDiag2D.cpp
  distribution/GaussianDiag3D.cpp
  distribution/GaussianDiag4D.cpp
  distribution/GaussianDiagND.cpp
  distribution/GaussianMixture.cpp
  distribution/GaussianND.cpp
  distribution/MotionDistribution.cpp
  dynamics/ContDynamicModel.cpp
  dynamics/ContDynamicRW.cpp
  dynamics/DiscDynamicModel.cpp
  filters/ICOND.cpp
  filters/ParticleFilter.cpp
  filters/WeightsVector.cpp
  imageutils/displayutils.cpp
  imageutils/histogrammes.cpp
  imageutils/imageutils.cpp
  imgproducer/ImgProducer.cpp
  likelihood/likelihoodutils.cpp
  likelihood/nok/Likelihood.cpp
  likelihood/nok/ShapeLikelihood.cpp
  miscelleanous/paramslist.cpp
  miscelleanous/RoiManager.cpp
  priors/PriorGauss.cpp
  priors/PriorGaussMixture.cpp
  priors/Sample.cpp
  shapedesc/shape.cpp
  shapedesc/ShapeModels.cpp
  shapedesc/shapeutils.cpp
  stringutils/stringutils.cpp
  targets/Target.cpp
  targets/TargetICU.cpp
  timeutils/temps_cpu.cpp
  twoDDetector/faceDetector.cpp
  twoDDetector/inputDetector.cpp
  twoDDetector/motionBlobDetector.cpp
  twoDDetector/motionDetector.cpp
  twoDDetector/skinBlobDetector.cpp
  twoDDetector/twoDDetector.cpp
  twoDDetector/HogPeopleDetector.cpp)

ADD_LIBRARY(LibPF SHARED ${SOURCES})
TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
  ${LIBPF_LIB}

  # OpenCV libs
  opencv_gpu
  opencv_highgui
  opencv_video
  opencv_flann
  opencv_objdetect
  opencv_imgproc
  opencv_ml
  opencv_legacy
  opencv_features2d
  opencv_calib3d
  opencv_core)

INSTALL(TARGETS LibPF DESTINATION ${CMAKE_INSTALL_LIBDIR})