[ $2 ] || {
	echo "args: <filename> <blur_radius>"
	return 1 2>/dev/null
	exit 1
}

filename=$1
blur_radius=$2

# remove extension from filename
filename=$(basename -- "$filename")
extension="${filename##*.}"
filename="${filename%.*}"

ffmpeg -y \
	-hide_banner \
	-loglevel error \
	-i "$1" \
	-vf boxblur=$blur_radius:$blur_radius \
	"$filename-blurred-$blur_radius.jpg"