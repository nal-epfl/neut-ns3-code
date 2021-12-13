/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universita' degli Studi di Napoli Federico II
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:  Stefano Avallone <stavallo@unina.it>
 */
#include <iostream>
#include <fstream>
#include <ns3/ipv4-header.h>
#include "ns3/string.h"
#include <ns3/internet-module.h>
#include "ns3/log.h"
#include "FifoQueueDiscCustom.h"
#include "ns3/object-factory.h"
#include "ns3/drop-tail-queue.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("FifoQueueDiscCustom");

NS_OBJECT_ENSURE_REGISTERED (FifoQueueDiscCustom);

TypeId FifoQueueDiscCustom::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FifoQueueDiscCustom")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<FifoQueueDiscCustom> ()
    .AddAttribute ("MaxSize",
                   "The max queue size",
                   QueueSizeValue (QueueSize ("1000p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
    .AddAttribute ("ResultsFolder", "folder to which all results are saved",
                    StringValue (""),
                    MakeStringAccessor(&FifoQueueDiscCustom::_resultsFolder),
                    MakeStringChecker())
  ;
  return tid;
}

FifoQueueDiscCustom::FifoQueueDiscCustom ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  NS_LOG_FUNCTION (this);
}

FifoQueueDiscCustom::~FifoQueueDiscCustom ()
{
  NS_LOG_FUNCTION (this);
}

bool
FifoQueueDiscCustom::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  if (GetCurrentSize () + item > GetMaxSize ())
    {
      NS_LOG_LOGIC ("Queue full -- dropping pkt");
      DropBeforeEnqueue (item, LIMIT_EXCEEDED_DROP);
      return false;
    }

  bool retval = GetInternalQueue (0)->Enqueue (item);
  _enquedEvents.push_back({item, retval, (Simulator::Now()).GetSeconds()});

  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return retval;
}

Ptr<QueueDiscItem>
FifoQueueDiscCustom::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();

  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  return item;
}

Ptr<const QueueDiscItem>
FifoQueueDiscCustom::DoPeek (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<const QueueDiscItem> item = GetInternalQueue (0)->Peek ();

  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  return item;
}

bool
FifoQueueDiscCustom::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("FifoQueueDiscCustom cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("FifoQueueDiscCustom needs no packet filter");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // add a DropTail queue
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                          ("MaxSize", QueueSizeValue (GetMaxSize ())));
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("FifoQueueDiscCustom needs 1 internal queue");
      return false;
    }

  return true;
}

void
FifoQueueDiscCustom::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
}

void FifoQueueDiscCustom::DoDispose() {
    std::ofstream outfile;
    outfile.open(_resultsFolder);
    for (FIFOEnqueueEvent event : _enquedEvents) {
        Ptr<const Ipv4QueueDiscItem> ipItem = DynamicCast<const Ipv4QueueDiscItem>(event.item);
        Ipv4Header ipHeader = ipItem->GetHeader();
        outfile << ipHeader.GetSource() << "," << ipHeader.GetDestination() << "," << ipItem->GetSize()
                << "," << event.isEnqueued << "," <<  event.time << std::endl;
    }
    outfile.close();
}

} // namespace ns3
