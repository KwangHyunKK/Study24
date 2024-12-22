#include <gtest/gtest.h>
#include <iostream>
#include "../src/Day241222.hpp"

TEST(Test241222, AsyncMesssageProcessing)
{
  using namespace D241222;
  Actor actor;

  auto future1 = actor.send_message([](){
    std::cout << "Message 1 processed.\n";
  });

  auto future2 = actor.send_message([](){
    std::cout << "Message 2 processed.\n";
  });

  EXPECT_NO_THROW(future1.get());
  EXPECT_NO_THROW(future2.get());
}

TEST(Test241222, AsyncMesssageProcessingwithSupervisor)
{
  using namespace D241222;
  Supervisor supervisor;
  int actor1 = supervisor.add_actor();
  int actor2 = supervisor.add_actor();

  auto future1 = supervisor.send_message_to_actor(actor1, [](){
    std::cout << "Message 1 processed.\n";
  });

  auto future2 = supervisor.send_message_to_actor(actor2, [](){
    std::cout << "Message 2 processed.\n";
  });

  EXPECT_NO_THROW(future1.get());
  EXPECT_NO_THROW(future2.get());

  supervisor.stop_actor(actor1);
  supervisor.stop_actor(actor2);
}

TEST(Test241222, AddAndSendMessages)
{
  using namespace D241222;
  Supervisor supervisor;
  int actor1 = supervisor.add_actor();

  auto future = supervisor.send_message_to_actor(actor1, [](){
    std::cout << "Supervisor message processed.\n";
  });

  EXPECT_NO_THROW(future.get());
  supervisor.stop_actor(actor1);
}

TEST(Test241222, StopInvalidActor)
{
  using namespace D241222;
  Supervisor supervisor;
  EXPECT_THROW(supervisor.send_message_to_actor(999, []() {}), std::runtime_error);
}

// In test, this case is true
// But this test cases don't demostrate async method call.