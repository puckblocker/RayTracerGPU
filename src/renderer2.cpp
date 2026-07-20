// RENDER
void Renderer::render(float *pixelBuffer, int resWidth, int resHeight, float &sampleCount)
{
    camera.camViewUpdate();

// FRAME BUFFER W/ PARALLELIZATION
// Loop Through Pixel Rows (Preserves i and j pixels)
#pragma omp parallel for collapse(2) schedule(dynamic, 1) // creates threads to run portions of the loop in parallel
    for (int j = 0; j < resHeight; j++)                   // render all pixels
    {
        for (int i = 0; i < resWidth; i++)
        {
            // ANTI-ALIASING (Multiple Rays)
            // Varaibles
            Ray ray;

            glm::vec3 color(0.0f);
            int smpleAmnt = 4; // samples per pixel
            float average = (float)smpleAmnt / (smpleAmnt + sampleCount);

            // Generate Jittered Rays (Jitter Happens Inside Ray)
            for (int index = 0; index < smpleAmnt; index++)
            {
                // Generate Ray
                ray = camera.rayGeneration(i, j);

                // Call Tracer
                glm::vec3 sampleColor = tracer(ray, 0);

                // NaN Check
                if (glm::any(glm::isnan(sampleColor)))
                {
                }
                else
                {
                    color += sampleColor;
                }
            }

            // Average Radiance Calculation (Monte Carlo)
            color = color / float(smpleAmnt);

            int index = (j * resWidth + i) * 3; // multiply by 3 to account for RGB components and resWidth to prevent overwriting pixels

            // PROGRESSIVE RENDERER (Average Total Samples)
            if (sampleCount > 0)
            {
                // color = ((pixelBufferTemp[index] * sampleCount) + color) / (sampleCount + 1.0f);
                pixelBuffer[index] = glm::mix(pixelBuffer[index], color.r, average);         // weighted red
                pixelBuffer[index + 1] = glm::mix(pixelBuffer[index + 1], color.g, average); // weighted green
                pixelBuffer[index + 2] = glm::mix(pixelBuffer[index + 2], color.b, average); // weighted blue
            }
            else
            {
                // Grab RGB Components
                pixelBuffer[index] = color.x;     // grab red value
                pixelBuffer[index + 1] = color.y; // grab green value
                pixelBuffer[index + 2] = color.z; // grab blue value
            }
        }
    }
}

// TRACER
glm::vec3 Renderer::tracer(Ray ray, unsigned int depth)
{
    glm::vec3 color = glm::vec3(0.0f);
    glm::vec3 specCoeff = glm::vec3(0.0f);

    // Check For Max Depth
    if (depth >= 5)
        return color;

    // First Intersect
    HitInfo hitInfo;
    hitInfo.valid = false;
    hitInfo.distance = 1000000.0f;

    // Check For Sphere
    HitInfo tempHit;
    int i = 0;
    for (i = 0; i < spheres.size(); i++)
    {
        tempHit = intersectSphere(ray, spheres[i], xForms);
        if (tempHit.valid && tempHit.distance < hitInfo.distance)
        {
            hitInfo = tempHit;
        }
    }

    // Check For Plane
    for (i = 0; i < planes.size(); i++)
    {
        tempHit = intersectPlane(ray, planes[i], xForms);
        if (tempHit.valid && tempHit.distance < hitInfo.distance)
        {
            hitInfo = tempHit;
        }
    }

    // Check For Triangle
    for (i = 0; i < triangles.size(); i++)
    {
        tempHit = intersectTriangle(ray, triangles[i], xForms);
        if (tempHit.valid && tempHit.distance < hitInfo.distance)
        {
            hitInfo = tempHit;
        }
    }

    // Non Light Hit
    if (hitInfo.valid && hitInfo.mat.emissive == false)
    {
        // VARIABLES
        glm::vec3 wiDirect;
        float pdf = 1.0f;
        float lightDist = 1.0f;

        // LIGHT SAMPLING & SHADOWS
        glm::vec3 Le;
        // Le = light.pointLight(pointLight, hitInfo, wiDirect, lightDist);
        // Le = light.directionalLight(directionalLight, hitInfo, wiDirect);

        if (glm::length(areaLight.color) > 0.0f)
        {
            Le = light.areaLight(areaLight, hitInfo, wiDirect, lightDist);
        }

        else if (glm::length(pointLight.color) > 0.0f)
        {
            Le = light.pointLight(pointLight, hitInfo, wiDirect, lightDist);
        }

        else if (glm::length(directionalLight.color) > 0.0f)
        {
            Le = light.directionalLight(directionalLight, hitInfo, wiDirect);
        }

        // Generate Shadows
        bool inShadow = false;
        Ray shadowRay;
        shadowRay.origin = hitInfo.point + (hitInfo.normal * 0.01f); // offset to avoid self shadowing
        shadowRay.direction = wiDirect;

        // Shadow Block Check Sphere
        HitInfo shadowHit;
        for (i = 0; i < spheres.size(); i++)
        {
            shadowHit = intersectSphere(shadowRay, spheres[i], xForms);
            if (shadowHit.valid && shadowHit.distance < lightDist && hitInfo.objID != shadowHit.objID) // Check for hit and behind object
                inShadow = true;
        }

        // Shadow Block Check Plane
        for (i = 0; i < planes.size(); i++)
        {
            shadowHit = intersectPlane(shadowRay, planes[i], xForms);
            if (shadowHit.valid && shadowHit.distance < lightDist && hitInfo.objID != shadowHit.objID)
                inShadow = true;
        }

        // COLOR / REFLECTANCE (BRDF)
        glm::vec3 R = hitInfo.mat.albedo;
        glm::vec3 w0 = -ray.direction;
        glm::vec3 directLight = glm::vec3(0.0f);

        // Not In Shadow
        if (!inShadow)
        {
            // DIRECT LIGHTING
            glm::vec3 rflctDirect = DirectBxDF(hitInfo, w0, wiDirect);

            float nDotWi = glm::dot(hitInfo.normal, wiDirect); // lambert's cos law
            directLight = rflctDirect * nDotWi * Le;
        }

        // INDIRECT LIGHT
        glm::vec3 wiIndirect;
        glm::vec3 rflctIndirect;

        if (hitInfo.mat.z == 0.0f)
        {
            rflctIndirect = BxDF(hitInfo, w0, wiIndirect, pdf);
        }
        else
        {
            rflctIndirect = LayeredBxDF(hitInfo, w0, wiIndirect, pdf);
        }

        Ray bounceRay;
        bounceRay.direction = wiIndirect;
        glm::vec3 normOffset = glm::dot(wiIndirect, hitInfo.normal) < 0.0f ? -hitInfo.normal : hitInfo.normal; // prevent self intersection
        bounceRay.origin = hitInfo.point + (normOffset * 0.002f);

        // Indirect Light Calculation
        glm::vec3 Li = tracer(bounceRay, depth + 1);

        // color += (rflct * pi) * Li;
        float nDotwi = glm::abs(glm::dot(hitInfo.normal, wiIndirect)); // lambert's cos law
        glm::vec3 indirectLight = (rflctIndirect * Li * nDotwi) / pdf;

        // COLOR CALCULATION
        color += indirectLight + directLight;

        // Max Radiance Clamp (Firefly fix (GREEN))
        float maxRad = 10.0f;
        color.x = glm::min(color.x, maxRad);
        color.y = glm::min(color.y, maxRad);
        color.z = glm::min(color.z, maxRad);

        return color;
    }
    // Exited Scene
    else
    {
        // Set Sky Color
        return glm::vec3(0.69f, 0.88f, 1.0f);

        // Add Emission From All Infinite Lights
        // return directionalLight.color;
    }
}
