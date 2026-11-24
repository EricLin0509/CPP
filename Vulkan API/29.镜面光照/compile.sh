#!/bin/bash

glslc -o simple_shader.frag.spv simple_shader.frag
glslc -o simple_shader.vert.spv simple_shader.vert

glslc -o point_light.frag.spv point_light.frag
glslc -o point_light.vert.spv point_light.vert
