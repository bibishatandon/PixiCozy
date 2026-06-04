# PowerShell setup script for PixiCozy dependencies

Write-Host "Creating project directory structure..."
New-Item -ItemType Directory -Force -Path "thirdparty" | Out-Null
New-Item -ItemType Directory -Force -Path "thirdparty/stb" | Out-Null
New-Item -ItemType Directory -Force -Path "assets/fonts" | Out-Null
New-Item -ItemType Directory -Force -Path "src" | Out-Null

$sdl2_url = "https://github.com/libsdl-org/SDL/releases/download/release-2.30.3/SDL2-devel-2.30.3-mingw.tar.gz"
$sdl2_ttf_url = "https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-devel-2.22.0-mingw.tar.gz"
$stb_url = "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h"
$font_url = "https://github.com/google/fonts/raw/main/ofl/nunito/Nunito-Bold.ttf"

# Download SDL2
if (-not (Test-Path "thirdparty/SDL2")) {
    Write-Host "Downloading SDL2 Development Package..."
    Invoke-WebRequest -Uri $sdl2_url -OutFile "thirdparty/sdl2.tar.gz"
    
    Write-Host "Extracting SDL2..."
    tar -xzf "thirdparty/sdl2.tar.gz" -C "thirdparty"
    
    # Rename to standard directory
    Move-Item -Path "thirdparty/SDL2-2.30.3" -Destination "thirdparty/SDL2"
    Remove-Item "thirdparty/sdl2.tar.gz"
    Write-Host "SDL2 ready."
} else {
    Write-Host "SDL2 already exists, skipping."
}

# Download SDL2_ttf
if (-not (Test-Path "thirdparty/SDL2_ttf")) {
    Write-Host "Downloading SDL2_ttf Development Package..."
    Invoke-WebRequest -Uri $sdl2_ttf_url -OutFile "thirdparty/sdl2_ttf.tar.gz"
    
    Write-Host "Extracting SDL2_ttf..."
    tar -xzf "thirdparty/sdl2_ttf.tar.gz" -C "thirdparty"
    
    # Rename to standard directory
    Move-Item -Path "thirdparty/SDL2_ttf-2.22.0" -Destination "thirdparty/SDL2_ttf"
    Remove-Item "thirdparty/sdl2_ttf.tar.gz"
    Write-Host "SDL2_ttf ready."
} else {
    Write-Host "SDL2_ttf already exists, skipping."
}

# Download stb_image_write.h
if (-not (Test-Path "thirdparty/stb/stb_image_write.h")) {
    Write-Host "Downloading stb_image_write.h..."
    Invoke-WebRequest -Uri $stb_url -OutFile "thirdparty/stb/stb_image_write.h"
    Write-Host "stb_image_write ready."
} else {
    Write-Host "stb_image_write already exists."
}

# Download Font
if (-not (Test-Path "assets/fonts/Nunito-Bold.ttf")) {
    Write-Host "Downloading Nunito-Bold font..."
    Invoke-WebRequest -Uri $font_url -OutFile "assets/fonts/Nunito-Bold.ttf"
    Write-Host "Nunito font ready."
} else {
    Write-Host "Nunito font already exists."
}

# Copy 32-bit DLLs to project root (for execution)
Write-Host "Copying runtime DLLs to root folder..."

if (Test-Path "thirdparty/SDL2/i686-w64-mingw32/bin") {
    Copy-Item -Path "thirdparty/SDL2/i686-w64-mingw32/bin/*.dll" -Destination "." -Force
}
if (Test-Path "thirdparty/SDL2_ttf/i686-w64-mingw32/bin") {
    Copy-Item -Path "thirdparty/SDL2_ttf/i686-w64-mingw32/bin/*.dll" -Destination "." -Force
}

# Also ensure build output directory has DLLs if it exists
New-Item -ItemType Directory -Force -Path "build" | Out-Null
if (Test-Path "thirdparty/SDL2/i686-w64-mingw32/bin") {
    Copy-Item -Path "thirdparty/SDL2/i686-w64-mingw32/bin/*.dll" -Destination "build" -Force
}
if (Test-Path "thirdparty/SDL2_ttf/i686-w64-mingw32/bin") {
    Copy-Item -Path "thirdparty/SDL2_ttf/i686-w64-mingw32/bin/*.dll" -Destination "build" -Force
}

Write-Host "All dependencies configured successfully! ✨"
