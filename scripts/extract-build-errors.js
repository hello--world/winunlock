#!/usr/bin/env node

/**
 * 从构建日志中提取关键错误信息
 * 识别编译错误、链接错误、CMake 错误等
 */

import fs from 'fs';

/**
 * 错误模式匹配规则
 */
const ERROR_PATTERNS = [
  // C++ 编译错误
  {
    name: 'C++ Compilation Error',
    pattern: /error:\s*(.+?)(?:\n|$)/gi,
    filePattern: /([^\s:]+\.(cpp|h|hpp|c)):(\d+):(\d+):/,
    severity: 'error'
  },
  // CMake 错误
  {
    name: 'CMake Error',
    pattern: /CMake Error[^:]*:\s*(.+?)(?:\n|$)/gi,
    filePattern: /CMakeLists\.txt[^:]*:(\d+)/,
    severity: 'error'
  },
  // CMake 警告（可能转换为错误）
  {
    name: 'CMake Warning',
    pattern: /CMake Warning[^:]*:\s*(.+?)(?:\n|$)/gi,
    severity: 'warning'
  },
  // 链接错误
  {
    name: 'Linker Error',
    pattern: /undefined reference|undefined symbol|ld:.*error|LINK\s+:\s+error/gi,
    severity: 'error'
  },
  // MSVC 编译错误
  {
    name: 'MSVC Error',
    pattern: /error C\d+:\s*(.+?)(?:\n|$)/gi,
    filePattern: /([^\s(]+)\((\d+)\)/,
    severity: 'error'
  },
  // 致命错误
  {
    name: 'Fatal Error',
    pattern: /fatal error:\s*(.+?)(?:\n|$)/gi,
    filePattern: /([^\s:]+\.(cpp|h|hpp|c)):(\d+):(\d+):/,
    severity: 'error'
  },
  // 未定义的引用
  {
    name: 'Undefined Reference',
    pattern: /undefined reference to [`'"]?([^`'"]+)[`'"]?/gi,
    severity: 'error'
  },
  // 找不到文件
  {
    name: 'File Not Found',
    pattern: /(?:cannot open|No such file|找不到文件)[^:]*:\s*([^\s\n]+)/gi,
    severity: 'error'
  },
  // 语法错误
  {
    name: 'Syntax Error',
    pattern: /(?:syntax error|expected|unexpected token)[^:]*:\s*(.+?)(?:\n|$)/gi,
    severity: 'error'
  }
];

/**
 * 提取错误信息
 */
function extractErrors(logText) {
  const errors = [];
  const seenErrors = new Set();

  for (const rule of ERROR_PATTERNS) {
    const matches = [...logText.matchAll(rule.pattern)];
    
    for (const match of matches) {
      const errorText = match[0].trim();
      const errorKey = `${rule.name}:${errorText.substring(0, 100)}`;
      
      // 避免重复
      if (seenErrors.has(errorKey)) {
        continue;
      }
      seenErrors.add(errorKey);

      const error = {
        type: rule.name,
        severity: rule.severity,
        message: errorText,
        fullMatch: match[0],
        file: null,
        line: null,
        column: null
      };

      // 尝试提取文件路径和行号
      if (rule.filePattern) {
        const fileMatch = errorText.match(rule.filePattern);
        if (fileMatch) {
          error.file = fileMatch[1];
          if (fileMatch[2]) {
            error.line = parseInt(fileMatch[2]);
          }
          if (fileMatch[3]) {
            error.column = parseInt(fileMatch[3]);
          }
        }
      }

      // 对于未定义的引用，提取函数名
      if (rule.name === 'Undefined Reference' && match[1]) {
        error.symbol = match[1];
      }

      errors.push(error);
    }
  }

  return errors;
}

/**
 * 提取关键错误行（包含错误的上下文）
 */
function extractErrorContext(logText, maxContext = 5) {
  const lines = logText.split('\n');
  const errorLines = [];
  const errorIndices = new Set();

  // 找到所有包含错误的行
  lines.forEach((line, index) => {
    for (const rule of ERROR_PATTERNS) {
      if (rule.pattern.test(line)) {
        errorIndices.add(index);
        // 添加上下文行
        for (let i = Math.max(0, index - maxContext); i <= Math.min(lines.length - 1, index + maxContext); i++) {
          errorIndices.add(i);
        }
      }
    }
  });

  // 提取相关行
  const sortedIndices = Array.from(errorIndices).sort((a, b) => a - b);
  sortedIndices.forEach(index => {
    errorLines.push({
      lineNumber: index + 1,
      content: lines[index]
    });
  });

  return errorLines;
}

/**
 * 生成结构化的错误报告
 */
function generateErrorReport(logText, options = {}) {
  const {
    includeContext = true,
    maxErrors = 50,
    maxContextLines = 10
  } = options;

  const errors = extractErrors(logText);
  const errorContext = includeContext ? extractErrorContext(logText, maxContextLines) : [];

  // 按严重程度和类型分组
  const groupedErrors = {
    errors: errors.filter(e => e.severity === 'error'),
    warnings: errors.filter(e => e.severity === 'warning'),
    byType: {}
  };

  errors.forEach(error => {
    if (!groupedErrors.byType[error.type]) {
      groupedErrors.byType[error.type] = [];
    }
    groupedErrors.byType[error.type].push(error);
  });

  // 限制错误数量
  const limitedErrors = errors.slice(0, maxErrors);

  return {
    summary: {
      totalErrors: groupedErrors.errors.length,
      totalWarnings: groupedErrors.warnings.length,
      errorTypes: Object.keys(groupedErrors.byType).length
    },
    errors: limitedErrors,
    groupedErrors,
    context: errorContext,
    rawLog: logText
  };
}

/**
 * 格式化错误报告为文本
 */
function formatErrorReport(report, format = 'text') {
  if (format === 'json') {
    return JSON.stringify(report, null, 2);
  }

  let output = '';
  
  // 摘要
  output += '='.repeat(60) + '\n';
  output += '构建错误摘要\n';
  output += '='.repeat(60) + '\n';
  output += `总错误数: ${report.summary.totalErrors}\n`;
  output += `总警告数: ${report.summary.totalWarnings}\n`;
  output += `错误类型数: ${report.summary.errorTypes}\n\n`;

  // 按类型分组显示
  output += '='.repeat(60) + '\n';
  output += '错误详情（按类型分组）\n';
  output += '='.repeat(60) + '\n\n';

  for (const [type, errors] of Object.entries(report.groupedErrors.byType)) {
    output += `\n【${type}】(${errors.length} 个)\n`;
    output += '-'.repeat(60) + '\n';
    
    errors.slice(0, 10).forEach((error, index) => {
      output += `${index + 1}. `;
      if (error.file) {
        output += `${error.file}`;
        if (error.line) {
          output += `:${error.line}`;
        }
        output += ' - ';
      }
      output += `${error.message}\n`;
    });
    
    if (errors.length > 10) {
      output += `... 还有 ${errors.length - 10} 个类似错误\n`;
    }
    output += '\n';
  }

  // 关键错误上下文
  if (report.context.length > 0) {
    output += '='.repeat(60) + '\n';
    output += '关键错误上下文\n';
    output += '='.repeat(60) + '\n\n';
    
    report.context.slice(0, 50).forEach(ctx => {
      output += `${ctx.lineNumber.toString().padStart(4, ' ')} | ${ctx.content}\n`;
    });
  }

  return output;
}

/**
 * 从日志文件或文本中提取错误
 */
async function extractBuildErrors(input, options = {}) {
  let logText;
  
  if (typeof input === 'string') {
    // 如果是文件路径
    if (fs.existsSync(input)) {
      logText = fs.readFileSync(input, 'utf-8');
    } else {
      // 否则作为日志文本
      logText = input;
    }
  } else {
    throw new Error('输入必须是文件路径或日志文本');
  }

  const report = generateErrorReport(logText, options);
  
  if (options.outputFile) {
    const formatted = formatErrorReport(report, options.format || 'text');
    fs.writeFileSync(options.outputFile, formatted);
    console.log(`✅ 错误报告已保存到: ${options.outputFile}`);
  }

  return report;
}

/**
 * 主函数
 */
async function main() {
  const args = process.argv.slice(2);
  const inputFile = args[0];
  const outputFile = args[1] || '.cursor/build-errors.txt';

  if (!inputFile) {
    console.error('用法: node extract-build-errors.js <日志文件或文本> [输出文件]');
    console.error('示例: node extract-build-errors.js workflow-log.txt .cursor/build-errors.txt');
    process.exit(1);
  }

  try {
    // 确保输出目录存在
    const outputDir = outputFile.substring(0, outputFile.lastIndexOf('/'));
    if (outputDir && !fs.existsSync(outputDir)) {
      fs.mkdirSync(outputDir, { recursive: true });
    }

    const report = await extractBuildErrors(inputFile, {
      outputFile,
      format: 'text',
      includeContext: true
    });

    console.log('\n📊 错误摘要:');
    console.log(`   总错误数: ${report.summary.totalErrors}`);
    console.log(`   总警告数: ${report.summary.totalWarnings}`);
    console.log(`   错误类型: ${report.summary.errorTypes}`);
    console.log(`\n✅ 详细报告已保存到: ${outputFile}`);
    
    return report;
  } catch (error) {
    console.error('❌ 提取错误时出错:', error.message);
    process.exit(1);
  }
}

// 运行主函数
if (import.meta.url === `file://${process.argv[1]}` || process.argv[1]?.endsWith('extract-build-errors.js')) {
  main().catch(error => {
    console.error('❌ 错误:', error.message);
    process.exit(1);
  });
}

export { extractBuildErrors, generateErrorReport, formatErrorReport, extractErrors };

