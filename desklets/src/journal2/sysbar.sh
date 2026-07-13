#!/bin/bash

BAR_WIDTH=30

bar() {
    local value=${1:-0}

    # clamp values
    [ "$value" -lt 0 ] && value=0
    [ "$value" -gt 100 ] && value=100

    local filled=$((value * BAR_WIDTH / 100))

    printf "["
    printf "%${filled}s" "" | tr " " "#"
    printf "%$((BAR_WIDTH-filled))s" "" | tr " " "-"
    printf "] %3d%%\n" "$value"
}

cpu_usage() {
    read -r _ user nice system idle iowait irq softirq steal _ _ < /proc/stat

    prev_idle=$idle
    prev_total=$((user+nice+system+idle+iowait+irq+softirq+steal))

    sleep 0.5

    read -r _ user nice system idle iowait irq softirq steal _ _ < /proc/stat

    idle=$idle
    total=$((user+nice+system+idle+iowait+irq+softirq+steal))

    diff_total=$((total-prev_total))
    diff_idle=$((idle-prev_idle))

    if [ "$diff_total" -eq 0 ]; then
        echo 0
    else
        echo $((100 - (diff_idle*100/diff_total)))
    fi
}

CPU=$(cpu_usage)

RAM=$(awk '
/MemTotal/ {total=$2}
/MemAvailable/ {avail=$2}
END {
    if (total>0) printf "%d", (total-avail)*100/total;
    else print 0
}' /proc/meminfo)

if command -v nvidia-smi >/dev/null 2>&1; then
    GPU=$(nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits | head -1)
    VRAM=$(nvidia-smi --query-gpu=memory.used,memory.total --format=csv,noheader,nounits | \
    awk -F',' '{gsub(/ /,""); if ($2>0) printf "%d", ($1*100/$2); else print 0}')
    TEMP=$(nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader,nounits | head -1)
else
    GPU=0
    VRAM=0
    TEMP=$(($(cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null)/1000))
fi

printf "%-5s" "CPU"
bar "$CPU"
echo

printf "%-5s" "GPU"
bar "$GPU"
echo

printf "%-5s" "VRAM"
bar "$VRAM"
echo

printf "%-5s" "RAM"
bar "$RAM"
echo

printf "%-5s %s°C\n" "TEMP" "$TEMP"
