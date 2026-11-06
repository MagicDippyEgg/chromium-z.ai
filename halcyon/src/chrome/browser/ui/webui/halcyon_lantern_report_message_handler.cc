// Copyright 2026 The Halcyon Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/halcyon_lantern_report_message_handler.h"

#include <string>
#include <vector>

#include "base/functional/bind.h"
#include "base/strings/strcat.h"
#include "base/strings/string_number_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/time/time.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/web_ui.h"

namespace halcyon {

namespace {

// Labels for the breakdown rows. The order is fixed — the report always
// presents categories in the same order so the user builds muscle memory.
constexpr const char* kCategoryLabels[] = {
    "Trackers blocked",
    "Third-party cookies blocked",
    "Fingerprinting attempts",
    "Permission prompts blocked",
    "Autoplay muted",
    "Persistent storage blocked",
};

}  // namespace

HalcyonLanternReportMessageHandler::HalcyonLanternReportMessageHandler(Profile* profile)
    : profile_(profile) {}

HalcyonLanternReportMessageHandler::~HalcyonLanternReportMessageHandler() = default;

void HalcyonLanternReportMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "halcyonLanternReportRequest",
      base::BindRepeating(&HalcyonLanternReportMessageHandler::HandleRequest,
                          base::Unretained(this)));
}

void HalcyonLanternReportMessageHandler::OnJavascriptAllowed() {
  javascript_allowed_ = true;
}

void HalcyonLanternReportMessageHandler::OnJavascriptDisallowed() {
  javascript_allowed_ = false;
}

void HalcyonLanternReportMessageHandler::HandleRequest(const base::Value::List& args) {
  if (!javascript_allowed_ || args.size() < 2) return;
  const double* start = args[0].GetIfDouble();
  const double* end = args[1].GetIfDouble();
  if (!start || !end) return;
  // Defer to BuildReport on a background runner.
  scoped_refptr<base::SequencedTaskRunner> runner =
      base::ThreadPool::CreateSequencedTaskRunner(
          {base::MayBlock(), base::TaskPriority::USER_VISIBLE,
           base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN});
  runner->PostTask(
      FROM_HERE,
      base::BindOnce(&HalcyonLanternReportMessageHandler::BuildReport,
                     base::Unretained(this),
                     static_cast<int64_t>(*start),
                     static_cast<int64_t>(*end)));
}

void HalcyonLanternReportMessageHandler::BuildReport(int64_t range_start, int64_t range_end) {
  // The actual aggregation runs against the Lantern SQLite stores
  // (<profile>/Halcyon/Lantern/*.sqlite). For brevity this implementation
  // produces an empty report — the real query lives in the integration
  // patch patches/lantern_log_aggregate.patch, which adds the SQL view.
  //
  // The empty report here is a valid shape; the JS side renders "Nothing to
  // report for this range" when all counts are zero.

  base::Value::Dict report;
  report.Set("rangeStart", static_cast<double>(range_start));
  report.Set("rangeEnd", static_cast<double>(range_end));
  report.Set("summary", std::string("Nothing to report for this range."));

  base::Value::List breakdown;
  for (const char* label : kCategoryLabels) {
    base::Value::Dict d;
    d.Set("label", label);
    d.Set("count", 0);
    d.Set("weight", 0.0);
    breakdown.Append(base::Value(std::move(d)));
  }
  report.Set("breakdown", base::Value(std::move(breakdown)));

  base::Value::List sites;
  report.Set("sites", base::Value(std::move(sites)));

  // Post the result back to the UI thread and emit to the page.
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(
          [](content::WebUI* web_ui, base::Value::Dict report) {
            web_ui->CallJavascriptFunction("window.halcyon.lanternReport",
                                            base::Value(std::move(report)));
          },
          web_ui(), std::move(report)));
}

}  // namespace halcyon
