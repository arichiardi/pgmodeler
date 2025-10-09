#!/bin/bash
# filepath: /home/raphael/pgmodeler/run-clang-tidy-fix.sh

set -e

PROJECT_ROOT="/home/raphael/pgmodeler"
BUILD_DIR="$PROJECT_ROOT/cmake-build"
COMPILE_DB="$BUILD_DIR/compile_commands.json"

cd "$PROJECT_ROOT"

if [ ! -f "$COMPILE_DB" ]; then
    echo "ERRO: compile_commands.json não encontrado em $BUILD_DIR"
    echo "Execute: cd cmake-build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .."
    exit 1
fi

CHECKS="modernize-use-equals-default,modernize-use-equals-delete,modernize-use-override"

echo "Enabled checks (with auto-fix):"
echo "    modernize-use-equals-default"
echo "    modernize-use-equals-delete"
echo "    modernize-use-override"
echo ""

# Coleta arquivos excluindo cmake-build e arquivos gerados
CPP_FILES=$(jq -r '.[].file' "$COMPILE_DB" | \
    grep -E '\.(cpp|cc)$' | \
    grep -v '/moc_' | \
    grep -v '/ui_' | \
    grep -v '/qrc_' | \
    grep -v '/cmake-build/' | \
    sort -u)

if [ -z "$CPP_FILES" ]; then
    echo "ERRO: Nenhum arquivo encontrado"
    exit 1
fi

FILE_COUNT=$(echo "$CPP_FILES" | wc -l)
echo "Running clang-tidy with auto-fix for $FILE_COUNT files..."
echo ""

COUNTER=0
for cpp_file in $CPP_FILES; do
    COUNTER=$((COUNTER + 1))
    echo "[$COUNTER/$FILE_COUNT] Fixing: $cpp_file"

    clang-tidy \
        -p="$BUILD_DIR" \
        --checks="$CHECKS" \
        --header-filter="^$PROJECT_ROOT/(libs|apps|plugins|priv-plugins)/.*\.h$" \
        --fix \
        --fix-errors \
        "$cpp_file" 2>&1 | grep -E "(warning:|error:|note:)" || true
done

echo ""
echo "Correções aplicadas em $FILE_COUNT arquivos!"
echo "IMPORTANTE: Revise as mudanças antes de commitar."
echo "Verifique especialmente:"
echo "  - Construtores/destrutores com = default"
echo "  - Métodos virtuais com override"
echo "  - Operadores com = delete"
