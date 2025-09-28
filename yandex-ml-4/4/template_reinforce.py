import numpy as np
import torch
import torch.nn as nn

def to_one_hot(y_tensor, ndims):
    """ helper: take an integer vector and convert it to 1-hot matrix. """
    y_tensor = y_tensor.type(torch.LongTensor).view(-1, 1)
    y_one_hot = torch.zeros(
        y_tensor.size()[0], ndims).scatter_(1, y_tensor, 1)
    return y_one_hot


def predict_probs(states, model):
    """
    Predict action probabilities given states.
    :param states: numpy array of shape [batch, state_shape]
    :param model: torch model
    :returns: numpy array of shape [batch, n_actions]
    """
    # convert states, compute logits, use softmax to get probability
    
    with torch.no_grad():
      tensor_ = torch.from_numpy(states).float()
      logits = model(tensor_)
      probs_tensor = torch.softmax(logits, dim=-1)
      probs = probs_tensor.numpy()

    assert probs is not None, "probs is not defined"

    return probs

def get_cumulative_rewards(rewards,  # rewards at each step
                           gamma=0.99  # discount for reward
                           ):
    """
    Take a list of immediate rewards r(s,a) for the whole session
    and compute cumulative returns (a.k.a. G(s,a) in Sutton '16).

    G_t = r_t + gamma*r_{t+1} + gamma^2*r_{t+2} + ...

    A simple way to compute cumulative rewards is to iterate from the last
    to the first timestep and compute G_t = r_t + gamma*G_{t+1} recurrently

    You must return an array/list of cumulative rewards with as many elements as in the initial rewards.
    """
    # YOUR CODE GOES HERE
    cumulative_rewards = []
    G_next = 0

    for i, reward in enumerate(reversed(rewards)):
      current_sum = reward + gamma * G_next
      G_next = current_sum
      cumulative_rewards.insert(0, current_sum)

    assert cumulative_rewards is not None, "cumulative_rewards is not defined"

    return cumulative_rewards

def get_loss(logits, actions, rewards, n_actions, gamma=0.99, entropy_coef=1e-2):
    """
    logits: torch.Tensor shape [T, n_actions]
    actions: list or array of ints length T
    rewards: list of immediate rewards length T
    returns: scalar torch.Tensor loss
    """
    # convert
    actions_t = torch.tensor(actions, dtype=torch.long)
    returns = np.array(get_cumulative_rewards(rewards, gamma))
    returns_t = torch.tensor(returns, dtype=torch.float32)

    # compute probabilities and log-probs (torch tensors)
    probs = torch.softmax(logits, dim=1)
    log_probs = torch.log_softmax(logits, dim=1)

    # select log-prob for each taken action
    # gather expects dims: (batch,1) -> then squeeze to [T]
    log_probs_for_actions = log_probs.gather(1, actions_t.view(-1, 1)).squeeze(1)

    # J_hat = (1/N) * sum_t log_pi(a_t|s_t) * G_t
    J_hat = torch.mean(log_probs_for_actions * returns_t)

    # entropy: per timestep H = - sum_a p(a) log p(a); use mean entropy
    entropy_per_t = -torch.sum(probs * log_probs, dim=1)
    entropy = torch.mean(entropy_per_t)

    # loss = -J_hat - entropy_coef * entropy
    loss = -J_hat - entropy_coef * entropy

    return loss